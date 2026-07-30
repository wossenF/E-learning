#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <sstream>
#include <iostream>

// Minimal HTTP server stub: parses request lines and headers, routes requests
// to handlers, serves simple static files (from repo), and uses a small
// thread-pool to simulate concurrency. This code is intended as a realistic
// substrate module to exercise parsing, I/O, and concurrency.

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string,std::string> headers;
    std::string body;
};

struct HttpResponse {
    int status = 200;
    std::map<std::string,std::string> headers;
    std::string body;
};

static std::string trim(const std::string& s) {
    size_t b=0, e=s.size(); while (b<e && isspace((unsigned char)s[b])) ++b; while (e>b && isspace((unsigned char)s[e-1])) --e; return s.substr(b,e-b);
}

bool parse_request(const std::string& raw, HttpRequest& req) {
    std::istringstream ss(raw);
    std::string line;
    if (!std::getline(ss, line)) return false;
    // request line: METHOD PATH VERSION
    std::istringstream rl(line);
    if (!(rl >> req.method >> req.path >> req.version)) return false;
    // headers
    while (std::getline(ss, line)) {
        if (line.empty() || line == "\r") break;
        size_t colon = line.find(':'); if (colon == std::string::npos) continue;
        std::string key = trim(line.substr(0,colon)); std::string val = trim(line.substr(colon+1));
        // normalize header key to lowercase
        for (auto &c : key) c = tolower((unsigned char)c);
        // strip trailing CR
        if (!val.empty() && val.back()=='\r') val.pop_back();
        req.headers[key] = val;
    }
    // read body if Content-Length present
    auto it = req.headers.find("content-length");
    if (it != req.headers.end()) {
        size_t len = std::stoul(it->second);
        std::string body; body.resize(len);
        ss.read(&body[0], (std::streamsize)len);
        req.body = body;
    }
    return true;
}

HttpResponse make_error_response(int status, const std::string& msg) {
    HttpResponse r; r.status = status; r.body = msg; r.headers["content-type"] = "text/plain"; r.headers["content-length"] = std::to_string(r.body.size()); return r;
}

HttpResponse serve_static_file(const std::string& docroot, const std::string& path) {
    // very small security check
    if (path.find("..") != std::string::npos) return make_error_response(403, "Forbidden");
    std::string file = docroot + "/" + (path.size()>1 && path[0]=='/' ? path.substr(1) : path);
    std::ifstream ifs(file, std::ios::binary);
    if (!ifs.good()) return make_error_response(404, "Not Found");
    std::ostringstream sb; sb << ifs.rdbuf(); std::string body = sb.str();
    HttpResponse r; r.status = 200; r.body = body; r.headers["content-type"] = "application/octet-stream"; r.headers["content-length"] = std::to_string(body.size()); return r;
}

using Handler = std::function<HttpResponse(const HttpRequest&)>;

class Router {
public:
    void add_route(const std::string& path_prefix, Handler h) { handlers_.push_back({path_prefix, h}); }
    HttpResponse route(const HttpRequest& req) const {
        for (const auto& p : handlers_) {
            if (req.path.rfind(p.first, 0) == 0) return p.second(req);
        }
        return make_error_response(404, "Not Found");
    }
private:
    std::vector<std::pair<std::string, Handler>> handlers_;
};

// tiny thread pool
class ThreadPool {
public:
    ThreadPool(size_t n): stop_(false) { for (size_t i=0;i<n;++i) threads_.emplace_back([this]{ worker(); }); }
    ~ThreadPool() { { std::unique_lock<std::mutex> lk(mu_); stop_ = true; cv_.notify_all(); } for (auto &t: threads_) if (t.joinable()) t.join(); }
    void submit(std::function<void()> fn) { { std::unique_lock<std::mutex> lk(mu_); q_.push(std::move(fn)); } cv_.notify_one(); }
private:
    void worker() {
        while (true) {
            std::function<void()> fn;
            { std::unique_lock<std::mutex> lk(mu_); cv_.wait(lk, [this]{ return stop_ || !q_.empty(); }); if (stop_ && q_.empty()) return; fn = std::move(q_.front()); q_.pop(); }
            try { fn(); } catch (...) { }
        }
    }
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> q_;
    std::mutex mu_;
    std::condition_variable cv_;
    bool stop_;
};

// simulate handling a connection: parse, route, and optionally serve static file
void handle_connection_simulated(const std::string& raw_request, const Router& router, const std::string& docroot) {
    HttpRequest req; if (!parse_request(raw_request, req)) { auto r = make_error_response(400, "Bad Request"); (void)r; return; }
    // special path for static files
    if (req.path.rfind("/static/",0) == 0) {
        auto resp = serve_static_file(docroot, req.path);
        (void)resp; // in real server we'd write to socket
        return;
    }
    auto resp = router.route(req);
    (void)resp;
}

// Example of wiring router and threadpool
void start_simulated_server_example() {
    Router r;
    r.add_route("/api/", [](const HttpRequest& req){ HttpResponse res; res.status=200; res.body = "ok"; res.headers["content-length"] = "2"; res.headers["content-type"] = "text/plain"; return res; });
    std::string docroot = "./";
    ThreadPool pool(4);
    // simulate some incoming requests
    for (int i=0;i<100;++i) {
        std::string req = "GET /api/test HTTP/1.1\r\nHost: localhost\r\n\r\n";
        pool.submit([req, &r, docroot]{ handle_connection_simulated(req, r, docroot); });
    }
}
