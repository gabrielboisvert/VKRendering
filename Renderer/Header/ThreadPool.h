#pragma once
#include <functional>
#include <queue>
#include <mutex>

namespace Renderer
{
    class ThreadPool {
        public:
            bool mShouldTerminate = false;
            std::mutex mQueueMutex;
            std::condition_variable mMutexCondition;
            std::vector<std::thread> mThreads;
            std::queue<std::function<void()>> mJobs;

            ThreadPool();
            void start();
            void queueJob(std::function<void()> pJob);
            void stop();
            bool busy();
            void threadLoop();
    };
}