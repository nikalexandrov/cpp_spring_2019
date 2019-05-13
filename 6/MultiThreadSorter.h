#pragma once

#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <queue>
#include <string>
#include <exception>

class MultiThreadSorter {
    std::string data;
    int threadNum;
    size_t memLim, threadMemLim, threadUintLim ;
    std::unique_ptr<uint64_t>  buf;
    std::ifstream dataStream;
    std::queue<std::string> outputFiles;
    std::vector<std::thread> threads;

    std::atomic<bool> finished = false;
    std::atomic<int> finishedStep = 0, finishedSort = 0;
    std::condition_variable cond;
    std::mutex sortFinishM, outQueueM, streamReadM, iterFinishM;

    void prepare(uint64_t * const buffer, const int id) {
        int file = 0;
        while (dataStream.good()) {
            std::unique_lock<std::mutex> lock(streamReadM);
            dataStream.read(reinterpret_cast<char *>(buffer), threadUintLim * sizeof(uint64_t));
            std::streamsize curSize = dataStream.gcount() / sizeof(uint64_t);
            lock.unlock();
            if (curSize != 0) {
                std::sort(buffer, buffer + curSize);
                std::string name = std::to_string(0) + '_' + std::to_string(id) + "_" + std::to_string(file) + ".bin";
                std::ofstream out(name, std::ios::binary);
                out.write(reinterpret_cast<char*>(buffer), curSize * sizeof(uint64_t));
                outputFiles.push(name);
                ++file;
            }
        }
    }

    void merge(const std::string & str1, const std::string & str2, uint64_t * const buf, const int id,const int iter,const int file) {
        std::ifstream f1(str1, std::ios::binary), f2(str2, std::ios::binary);
        const std::string name = std::to_string(iter) + '_' + std::to_string(id) + '_' + std::to_string(file) + ".bin";
        std::ofstream out(name, std::ios::binary);

        const size_t lim = threadUintLim / 4, limM = 2 * lim;
        uint64_t* const bufL = buf, * const bufR = buf + lim, * const bufM = buf + limM;

        f1.read(reinterpret_cast<char*>(bufL), lim * sizeof(uint64_t));
        size_t readLeft = f1.gcount() / sizeof(uint64_t);
        f2.read(reinterpret_cast<char*>(bufR), lim * sizeof(uint64_t));
        size_t readRight = f2.gcount() / sizeof(uint64_t);

        size_t left = 0, middle = 0, right = 0;

        while (!f1.eof() || !f2.eof() || left < readLeft || right < readRight) {
            if (left == readLeft && !f1.eof()) {
                f1.read(reinterpret_cast<char*>(bufL), lim * sizeof(uint64_t));
                readLeft = f1.gcount() / sizeof(uint64_t);
                left = 0;
            }
            if (right == readRight && !f2.eof()) {
                f2.read(reinterpret_cast<char*>(bufR), lim * sizeof(uint64_t));
                readRight = f2.gcount() / sizeof(uint64_t);
                right = 0;
            }
            if (middle == limM) {
                out.write(reinterpret_cast<char*>(bufM), middle * sizeof(uint64_t));
                middle = 0;
            }

            if (left < readLeft && right < readRight) {
                if (bufR[right] < bufL[left]) {
                    bufM[middle] = bufR[right];
                    ++right;
                } else {
                    bufM[middle] = bufL[left];
                    ++left;
                }
                ++middle;
            } else if (left == readLeft && right < readRight) {
                bufM[middle] = bufR[right];
                ++middle;
                ++right;
            } else if (right == readRight && left < readLeft) {
                bufM[middle] = bufL[left];
                ++left;
                ++middle;
            }
        }

        out.write(reinterpret_cast<char*>(bufM), middle * sizeof(uint64_t));
        std::unique_lock<std::mutex> qlock(outQueueM);
        outputFiles.push(name);
    }

    void threadSorter(const int id) {
        uint64_t * const locBuf = buf.get() + id*threadUintLim;
        int iter = 0, file = 0;
        
        prepare(locBuf, id);
        ++iter;

        std::unique_lock<std::mutex> lock(iterFinishM);
        ++finishedStep;

        cond.notify_all();
        while (finishedStep < threadNum)
            cond.wait(lock);
        lock.unlock();
            
        while (outputFiles.size() >= 2) {
            std::unique_lock<std::mutex> qLock(outQueueM);
            if (outputFiles.size() >= 2) {
                std::string tmp1 = outputFiles.front();
                outputFiles.pop();
                std::string tmp2 = outputFiles.front();
                outputFiles.pop();
                qLock.unlock();
                merge(tmp1, tmp2, locBuf, id, iter, file);
                ++file;
            }
        }

        std::unique_lock<std::mutex> fLock(sortFinishM);
        ++finishedSort;
        
        if (finishedSort == threadNum) {
            if (outputFiles.empty())
                throw std::logic_error("No output files found");
            else std::cout << "Finished, result in file: " << outputFiles.front() << std::endl;
        }
    }
public:
    MultiThreadSorter (std::string filename, int threadCount, size_t memoryLimit) :
        data (filename),
        threadNum (threadCount),
        memLim (memoryLimit),
        threadMemLim (memoryLimit / threadCount),
        threadUintLim (memoryLimit / (threadCount * sizeof(uint64_t))),
        buf (new uint64_t[memLim / sizeof(uint64_t)]),
        dataStream(data, std::ios::binary) {}
    void sort () {
        for (int i = 0; i < threadNum; ++i)
            threads.emplace_back(&MultiThreadSorter::threadSorter, this, i);
        for (int i = 0; i < threadNum; ++i)
            threads[i].join();
    }
};