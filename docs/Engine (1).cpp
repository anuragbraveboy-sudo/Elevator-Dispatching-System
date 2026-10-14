#include "elev/Engine.hpp"

#include <algorithm>
#include <cstdlib>

namespace elev {

// ---------------------------------------------------------------- strategy

int NearestStrategy::select(const Request& r, const std::vector<ElevatorView>& eligible) const {
    int best = -1;
    int bestDist = 0;
    size_t bestStops = 0;
    for (const auto& e : eligible) {
        int d = std::abs(e.floor - r.source);
        bool better = best < 0 || d < bestDist || (d == bestDist && e.stops < bestStops);
        // ties on (distance, stops) keep the lower id because we scan in id order
        if (better) {
            best = e.id;
            bestDist = d;
            bestStops = e.stops;
        }
    }
    return best;
}

// ------------------------------------------------------------- lifecycle

Engine::Engine(Config cfg, std::unique_ptr<DispatchStrategy> strategy, EventSink sink)
    : cfg_(cfg), strategy_(std::move(strategy)), sink_(std::move(sink)) {
    if (cfg_.floors < 1 || cfg_.elevators < 1 || cfg_.capacity < 1)
        throw std::invalid_argument("floors, elevators and capacity must be >= 1");
    if (!strategy_) throw std::invalid_argument("strategy required");
    for (int i = 0; i < cfg_.elevators; ++i) {
        Elev e;
        e.id = i;
        e.capacity = cfg_.capacity;
        elevs_.push_back(e);
    }
    worker_ = std::thread([this] { run(); });
}

Engine::~Engine() {
    {
        std::lock_guard<std::mutex> lk(qmu_);
        stopping_ = true;
    }
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void Engine::run() {
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lk(qmu_);
            cv_.wait(lk, [this] { return stopping_ || !q_.empty(); });
            if (q_.empty()) return;  // stopping and fully drained
            job = std::move(q_.front());
            q_.pop();
        }
        job();  // lock NOT held while mutating state or calling the sink
    }
}

// ----------------------------------------------------------- public API

std::future<SubmitResult> Engine::submit(SubmitInput in) {
    return post([this, in = std::move(in)] { return doSubmit(in); });
}
std::future<bool> Engine::cancel(uint64_t id) {
    return post([this, id] { return doCancel(id); });
}
std::future<void> Engine::tick() {
    return post([this] { doTick(); });
}
std::future<bool> Engine::setMaintenance(int id, bool on) {
    return post([this, id, on] { return doMaintenance(id, on); });
}
std::future<void> Engine::setStrategy(std::unique_ptr<DispatchStrategy> s) {
    auto holder = std::make_shared<std::unique_ptr<DispatchStrategy>>(std::move(s));
    return post([this, holder] {
        if (!*holder) throw std::invalid_argument("strategy required");
        strategy_ = std::move(*holder);
        dispatchPending();  // pending requests are re-evaluated under the new strategy
    });
}
std::future<Snapshot> Engine::snapshot() {
    return post([this] {
        Snapshot s;
        s.tick = tick_;
        s.seq = seq_;
        for (const auto& kv : reqs_) s.requests.push_back(kv.second);
        for (const auto& e : elevs_) s.elevators.push_back(view(e));
        return s;
    });
}

// ------------------------------------------------- scheduler-thread only

ElevatorView Engine::view(const Elev& e) const {
    return ElevatorView{e.id, e.floor, e.load, e.capacity, e.dir, e.stops.size(), e.available};
}

void Engine::emit(std::string type, uint64_t entityId, RequestStatus from, RequestStatus to,
                  int elev) {
    Event ev{++seq_, tick_, std::move(type), entityId, from, to, elev};
    if (sink_) sink_(ev);
}

// The ONLY place a request's status changes. An illegal move is an engine bug,
// so it throws instead of silently corrupting the lifecycle.
void Engine::transition(Request& r, RequestStatus to, int elevatorId) {
    if (!isValidTransition(r.status, to))
        throw std::logic_error(std::string("illegal request transition ") + toString(r.status) +
                               " -> " + toString(to));
    RequestStatus from = r.status;
    r.status = to;
    r.elevatorId = elevatorId;
    ++r.version;
    emit("request:status", r.id, from, to, elevatorId);
}

bool Engine::tryAssign(Request& r) {
    std::vector<ElevatorView> eligible;
    for (const auto& e : elevs_)
        if (e.available && e.load + r.passengers <= e.capacity) eligible.push_back(view(e));
    if (eligible.empty()) return false;
    int id = strategy_->select(r, eligible);
    if (id < 0 || id >= static_cast<int>(elevs_.size())) return false;
    Elev& e = elevs_[id];
    transition(r, RequestStatus::ASSIGNED, id);  // single writer => cannot be assigned twice
    e.assigned.push_back(r.id);
    addStop(e, r.source);
    return true;
}

void Engine::dispatchPending() {
    std::set<uint64_t> snapshot = pending_;  // iterate a copy; oldest id first
    for (uint64_t id : snapshot) {
        Request& r = reqs_.at(id);
        if (r.status != RequestStatus::PENDING) {
            pending_.erase(id);
            continue;
        }
        if (tryAssign(r)) pending_.erase(id);
    }
}

void Engine::addStop(Elev& e, int floor) {
    if (std::find(e.stops.begin(), e.stops.end(), floor) == e.stops.end()) e.stops.push_back(floor);
}

void Engine::pruneStop(Elev& e, int floor) {
    for (uint64_t id : e.assigned)
        if (reqs_.at(id).source == floor) return;
    for (uint64_t id : e.onboard)
        if (*reqs_.at(id).destination == floor) return;
    e.stops.erase(std::remove(e.stops.begin(), e.stops.end(), floor), e.stops.end());
}

void Engine::serve(Elev& e) {
    e.stops.erase(std::remove(e.stops.begin(), e.stops.end(), e.floor), e.stops.end());

    // 1) passengers alight
    for (auto it = e.onboard.begin(); it != e.onboard.end();) {
        Request& r = reqs_.at(*it);
        if (*r.destination == e.floor) {
            e.load -= r.passengers;
            transition(r, RequestStatus::COMPLETED, e.id);
            it = e.onboard.erase(it);
        } else {
            ++it;
        }
    }

    // 2) assigned requests at this floor board, if capacity allows
    std::vector<uint64_t> waiting;
    waiting.swap(e.assigned);
    bool requeued = false;
    for (uint64_t id : waiting) {
        Request& r = reqs_.at(id);
        if (r.source != e.floor) {
            e.assigned.push_back(id);
            continue;
        }
        if (e.load + r.passengers > e.capacity) {
            // capacity is never exceeded: bounce back to the pending queue
            transition(r, RequestStatus::PENDING, -1);
            pending_.insert(id);
            requeued = true;
            continue;
        }
        transition(r, RequestStatus::IN_PROGRESS, e.id);
        if (r.destination) {
            e.load += r.passengers;
            e.onboard.push_back(id);
            addStop(e, *r.destination);
        } else {
            // A hall call carries no destination: the call is serviced on pickup.
            // The passengers' destination arrives later as a new request.
            transition(r, RequestStatus::COMPLETED, e.id);
        }
    }
    if (requeued) dispatchPending();
}

SubmitResult Engine::doSubmit(const SubmitInput& in) {
    auto fail = [](std::string msg) {
        SubmitResult r;
        r.error = std::move(msg);
        return r;
    };
    if (!in.idempotencyKey.empty()) {
        auto it = idem_.find(in.idempotencyKey);
        if (it != idem_.end()) {
            SubmitResult r;
            r.ok = true;
            r.duplicate = true;
            r.id = it->second;
            return r;
        }
    }
    if (in.source < 0 || in.source >= cfg_.floors) return fail("source floor out of range");
    if (in.passengers < 1 || in.passengers > cfg_.capacity)
        return fail("passenger count must be between 1 and elevator capacity");

    Direction dir = in.direction;
    if (in.destination) {
        int d = *in.destination;
        if (d < 0 || d >= cfg_.floors) return fail("destination floor out of range");
        if (d == in.source) return fail("destination equals source");
        dir = d > in.source ? Direction::UP : Direction::DOWN;
    } else {
        if (dir != Direction::UP && dir != Direction::DOWN)
            return fail("hall call requires direction UP or DOWN");
        if (dir == Direction::UP && in.source == cfg_.floors - 1)
            return fail("cannot call UP from the top floor");
        if (dir == Direction::DOWN && in.source == 0) return fail("cannot call DOWN from floor 0");
    }

    Request r;
    r.id = nextId_++;
    r.source = in.source;
    r.destination = in.destination;
    r.direction = dir;
    r.passengers = in.passengers;
    r.priority = in.priority;
    r.createdTick = tick_;
    reqs_[r.id] = r;
    pending_.insert(r.id);
    if (!in.idempotencyKey.empty()) idem_[in.idempotencyKey] = r.id;
    emit("request:created", r.id, RequestStatus::PENDING, RequestStatus::PENDING, -1);
    dispatchPending();

    SubmitResult out;
    out.ok = true;
    out.id = r.id;
    return out;
}

bool Engine::doCancel(uint64_t id) {
    auto it = reqs_.find(id);
    if (it == reqs_.end()) return false;
    Request& r = it->second;
    if (r.status == RequestStatus::PENDING) {
        pending_.erase(id);
        transition(r, RequestStatus::CANCELLED, -1);
        return true;
    }
    if (r.status == RequestStatus::ASSIGNED) {
        Elev& e = elevs_[r.elevatorId];
        e.assigned.erase(std::remove(e.assigned.begin(), e.assigned.end(), id), e.assigned.end());
        int src = r.source;
        transition(r, RequestStatus::CANCELLED, -1);
        pruneStop(e, src);
        return true;
    }
    return false;  // already riding or finished: cannot cancel
}

void Engine::doTick() {
    ++tick_;
    for (auto& e : elevs_) {
        if (!e.available) continue;
        if (e.stops.empty()) {
            e.dir = Direction::IDLE;
            continue;
        }
        int target = e.stops.front();
        if (e.floor == target) {
            serve(e);
            continue;
        }
        e.dir = target > e.floor ? Direction::UP : Direction::DOWN;
        e.floor += (e.dir == Direction::UP) ? 1 : -1;
        emit("elevator:position", static_cast<uint64_t>(e.id), RequestStatus::PENDING,
             RequestStatus::PENDING, e.floor);
        if (e.floor == target) serve(e);
    }
    dispatchPending();  // freed capacity may let waiting requests be assigned
}

bool Engine::doMaintenance(int id, bool on) {
    if (id < 0 || id >= static_cast<int>(elevs_.size())) return false;
    Elev& e = elevs_[id];
    if (on) {
        if (!e.available) return true;  // idempotent
        e.available = false;
        e.dir = Direction::IDLE;
        // Not-yet-boarded requests are safe to move: back to PENDING, reassigned below.
        for (uint64_t rid : e.assigned) {
            transition(reqs_.at(rid), RequestStatus::PENDING, -1);
            pending_.insert(rid);
        }
        // Riders are NOT silently transferred: software cannot move trapped passengers.
        for (uint64_t rid : e.onboard) transition(reqs_.at(rid), RequestStatus::FAILED, e.id);
        e.assigned.clear();
        e.onboard.clear();
        e.stops.clear();
        e.load = 0;
        emit("elevator:maintenance", static_cast<uint64_t>(id), RequestStatus::PENDING,
             RequestStatus::PENDING, 1);
    } else {
        if (e.available) return true;
        e.available = true;
        emit("elevator:maintenance", static_cast<uint64_t>(id), RequestStatus::PENDING,
             RequestStatus::PENDING, 0);
    }
    dispatchPending();
    return true;
}

}  // namespace elev
