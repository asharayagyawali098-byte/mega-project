#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

// ============================================================================
// MEMORY TRACKER & PROFILING UTILITIES
// ============================================================================
struct AllocationMetrics {
    uint32_t TotalAllocated = 0;
    uint32_t TotalFreed = 0;
    uint32_t CurrentUsage() const { return TotalAllocated - TotalFreed; }
};

static AllocationMetrics s_AllocationMetrics;

void* operator new(size_t size) {
    s_AllocationMetrics.TotalAllocated += static_cast<uint32_t>(size);
    return malloc(size);
}

void operator delete(void* memory, size_t size) noexcept {
    s_AllocationMetrics.TotalFreed += static_cast<uint32_t>(size);
    free(memory);
}

// ============================================================================
// MATHEMATICAL & DATA ANALYTICS CORE
// ============================================================================
namespace SystemAnalytics {

    class MathematicalEngine {
    public:
        static double CalculateMean(const std::vector<double>& data) {
            if (data.empty()) return 0.0;
            double sum = std::accumulate(data.begin(), data.end(), 0.0);
            return sum / data.size();
        }

        static double CalculateStandardDeviation(const std::vector<double>& data) {
            if (data.size() <= 1) return 0.0;
            double mean = CalculateMean(data);
            double accum = 0.0;
            std::for_each(data.begin(), data.end(), [&](const double d) {
                accum += (d - mean) * (d - mean);
            });
            return std::sqrt(accum / (data.size() - 1));
        }

        static std::vector<double> FastFourierTransformSimulation(size_t sampleSize) {
            std::vector<double> samples;
            samples.reserve(sampleSize);
            for (size_t i = 0; i < sampleSize; ++i) {
                double val = std::sin(i * 0.05) + (static_cast<double>(rand() % 100) / 100.0);
                samples.push_back(val);
            }
            return samples;
        }
    };

    // ============================================================================
    // CONCURRENCY & THREAD-POOL BENCHMARKING
    // ============================================================================
    class ParallelProcessor {
    private:
        std::mutex m_Mutex;
        std::vector<std::string> m_Results;

    public:
        void ProcessTask(int taskId, int durationMs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::ostringstream ss;
            ss << "[THREAD-" << std::this_thread::get_id() << "] Task #" << taskId 
               << " completed processing in " << durationMs << "ms.";
            m_Results.push_back(ss.str());
        }

        void RunParallelWorkload(int taskCount) {
            std::vector<std::thread> workers;
            for (int i = 0; i < taskCount; ++i) {
                int duration = 20 + (i * 10);
                workers.emplace_back(&ParallelProcessor::ProcessTask, this, i + 1, duration);
            }

            for (auto& worker : workers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        void PrintResults() const {
            for (const auto& res : m_Results) {
                std::cout << "  " << res << "\n";
            }
        }
    };
}

// ============================================================================
// SYSTEM ACCOUNT MANAGER & PIPELINE
// ============================================================================
class AccountDataModel {
public:
    std::string ServiceName;
    std::string AccountHandle;
    uint32_t MetricValue;

    AccountDataModel(std::string name, std::string handle, uint32_t val)
        : ServiceName(std::move(name)), AccountHandle(std::move(handle)), MetricValue(val) {}
};

class PipelineEngine {
private:
    std::vector<std::unique_ptr<AccountDataModel>> m_Accounts;

public:
    void RegisterAccount(const std::string& service, const std::string& handle, uint32_t metric) {
        m_Accounts.push_back(std::make_unique<AccountDataModel>(service, handle, metric));
    }

    void ProcessPipeline() {
        std::cout << "\n=======================================================\n";
        std::cout << "  C++ HIGH-PERFORMANCE DATA PIPELINE EXECUTION         \n";
        std::cout << "=======================================================\n";

        for (const auto& acc : m_Accounts) {
            std::cout << " [PIPELINE] Service: " << std::setw(12) << std::left << acc->ServiceName 
                      << " | Handle: " << std::setw(18) << acc->AccountHandle 
                      << " | Sync Score: " << acc->MetricValue << " pts\n";
        }
    }
};

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main() {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "Starting C++ Analytics Engine...\n";
    
    // 1. Data Pipeline
    PipelineEngine pipeline;
    pipeline.RegisterAccount("YouTube", "VoltCutz Editz", 9850);
    pipeline.RegisterAccount("Microsoft", "ashraya@outlook.com", 10000);
    pipeline.RegisterAccount("Gmail", "ashraya.dev@gmail.com", 9920);
    pipeline.RegisterAccount("Instagram", "@voltcutz.editz", 8740);
    pipeline.RegisterAccount("Facebook", "Ashraya Gyawali", 8120);
    pipeline.ProcessPipeline();

    // 2. Math & Signal Analytics
    std::cout << "\n[MATH] Generating 1,000 Data Samples for FFT Analysis...\n";
    auto samples = SystemAnalytics::MathematicalEngine::FastFourierTransformSimulation(1000);
    double mean = SystemAnalytics::MathematicalEngine::CalculateMean(samples);
    double stdDev = SystemAnalytics::MathematicalEngine::CalculateStandardDeviation(samples);

    std::cout << " [RESULTS] Data Mean: " << std::fixed << std::setprecision(4) << mean << "\n";
    std::cout << " [RESULTS] Std Dev  : " << stdDev << "\n";

    // 3. Concurrency Workload
    std::cout << "\n[CONCURRENCY] Spawning 4 Parallel Worker Threads...\n";
    SystemAnalytics::ParallelProcessor processor;
    processor.RunParallelWorkload(4);
    processor.PrintResults();

    // 4. Memory Profiling Summary
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    std::cout << "\n=======================================================\n";
    std::cout << "  ENGINE PERFORMANCE SUMMARY                           \n";
    std::cout << "=======================================================\n";
    std::cout << " Total Time Elapsed : " << elapsed.count() << " ms\n";
    std::cout << " Memory Allocated   : " << s_AllocationMetrics.TotalAllocated << " bytes\n";
    std::cout << " Memory Freed       : " << s_AllocationMetrics.TotalFreed << " bytes\n";
    std::cout << " Active Allocations : " << s_AllocationMetrics.CurrentUsage() << " bytes\n";
    std::cout << " Engine Status      : SUCCESSFUL EXIT (0)\n";
    std::cout << "=======================================================\n";

    return 0;
}