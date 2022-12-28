#include <iostream>
#include <fstream>
#include <iomanip> // for std::setprecision()
#include <string>  // for std::stoi()
#include <random>
#include <cmath>
#include <numbers>
#include <exception>
#include <chrono>
#include <filesystem>
#include <vector>
#include <utility> // for std::pair
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <unordered_map>

#include <stdlib.h> // for EXIT_{SUCCESS, FAILURE}

namespace
{
    using Pair = std::pair<double, double>;
    using Clock = std::chrono::steady_clock;
    using Seconds = std::chrono::duration<double>;
    using TimingsMap = std::unordered_map<std::string, Seconds>;
    namespace fs = std::filesystem;

}

class Profiler
{
public:
    Profiler(const std::string &tag,
             TimingsMap &timings)
        : _tag(tag),
          _start(Clock::now()),
          _timings(timings)
    {
    }

    ~Profiler()
    {
        auto end = Clock::now();
        Seconds duration = end - _start;
        _timings[_tag] = duration;
    }

private:
    std::string _tag;
    Clock::time_point _start;
    TimingsMap &_timings;
};

void count_points_in_circle(const std::vector<Pair> &points, int low, int high,
                            std::atomic<int> &num_in_circle)
{
    int my_num_in_circle = 0;
    constexpr double unit_circle_radius = 1.0;

    for (int i = low; i < high; ++i)
    {
        const double &x = points[i].first;
        const double &y = points[i].second;

        const double distance_from_origin = std::sqrt(x * x + y * y);
        const bool in_circle = distance_from_origin <= unit_circle_radius;
        if (in_circle)
        {
            ++my_num_in_circle;
        }
    }

    num_in_circle.fetch_add(my_num_in_circle, std::memory_order_relaxed);
}

int main(int argc, char **argv)
{
    constexpr int seed = 1;
    constexpr int max_threads = 8;

    constexpr auto extension = ".csv";
    constexpr char separator = ',';
    constexpr int precision = 15;

    int num_threads;
    int num_points;
    std::ostringstream oss;
    TimingsMap timings;
    {
        Profiler main_profiler("main", timings);

        if (argc != 3)
        {
            std::cout << "usage: " << fs::path(argv[0]).filename().string()
                      << " <num_threads> <num_points>" << std::endl;
            return EXIT_FAILURE;
        }

        try
        {
            num_threads = std::stoi(argv[1]);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        if (num_threads <= 0)
        {
            std::cerr << "num_threads must be positive" << std::endl;
            return EXIT_FAILURE;
        }
        else if (num_threads > max_threads)
        {
            std::cerr << "don't use more than " << max_threads << " threads"
                      << std::endl;
            return EXIT_FAILURE;
        }

        try
        {
            num_points = std::stoi(argv[2]);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        if (num_points <= 0)
        {
            std::cerr << "num_points must be positive" << std::endl;
            return EXIT_FAILURE;
        }
        else if (num_points % num_threads != 0)
        {
            std::cerr << "num_points must be divisible by " << num_threads
                      << std::endl;
            return EXIT_FAILURE;
        }

        oss << "points_" << num_points << extension;
        const std::string points_file_name = oss.str();

        fs::path points_path = fs::current_path().append(points_file_name);
        std::ofstream points_file(points_path.string());
        if (!points_file)
        {
            std::cerr << "cannot open " << points_path << " for writing"
                      << std::endl;
            return EXIT_FAILURE;
        }

        std::mt19937_64 generator(seed); // 64-bit Mersenne Twister
        std::uniform_real_distribution<> uniform(-1.0, 1.0);

        std::vector<Pair> points;
        points.reserve(num_points);
        points_file << "x" << separator << "y" << std::endl;
        for (int i = 0; i < num_points; ++i)
        {
            const double x = uniform(generator);
            const double y = uniform(generator);

            points_file << std::setprecision(precision)
                        << x << separator << y << std::endl;

            points.emplace_back(std::make_pair(x, y));
        }

        const int points_per_thread = num_points / num_threads;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        std::atomic<int> num_in_circle = 0;
        for (int i = 0; i < num_threads; ++i)
        {
            const int low = i * points_per_thread;
            const int high = (i + 1) * points_per_thread;
            threads.emplace_back(std::thread(count_points_in_circle,
                                             std::ref(points), low, high,
                                             std::ref(num_in_circle)));
        }
        for (auto &t : threads)
        {
            t.join();
        }

        // pi*r^2 / 4*r^2 = pi / 4; use 4.0 to avoid integer division
        const double pi_estimate = 4.0 * num_in_circle / num_points;
        constexpr double pi_actual = std::numbers::pi_v<double>;
        const double error = pi_estimate - pi_actual;

        oss.str("");
        oss << "errors" << extension;
        const std::string errors_file_name = oss.str();

        // only write header the first time we open the file
        fs::path errors_path = fs::current_path().append(errors_file_name);
        const bool write_errors_header = !fs::exists(errors_path);

        // accumulate errors across runs
        std::ofstream errors_file;
        errors_file.open(errors_path.string(), std::ios_base::app);
        if (!errors_file)
        {
            std::cerr << "cannot open " << errors_path << " for writing"
                      << std::endl;
            return EXIT_FAILURE;
        }

        if (write_errors_header)
        {
            errors_file << "num_points" << separator
                        << "pi_estimate" << separator
                        << "error" << std::endl;
        }

        errors_file << std::setprecision(precision)
                    << num_points << separator
                    << pi_estimate << separator
                    << error << std::endl;
    }

    oss.str("");
    oss << "timings" << extension;
    const std::string timings_file_name = oss.str();

    // only write header the first time we open the file
    fs::path timings_path = fs::current_path().append(timings_file_name);
    const bool write_timings_header = !fs::exists(timings_path);

    // accumulate timings across runs
    std::ofstream timings_file;
    timings_file.open(timings_path.string(), std::ios_base::app);
    if (!timings_file)
    {
        std::cerr << "cannot open " << timings_path << " for writing"
                  << std::endl;
        return EXIT_FAILURE;
    }

    if (write_timings_header)
    {
        timings_file << "num_points" << separator
                     << "main_seconds" << std::endl;
    }

    Seconds main_seconds = timings["main"];
    timings_file << std::setprecision(precision)
                 << num_points << separator
                 << main_seconds.count() << std::endl;

    return EXIT_SUCCESS;
}
