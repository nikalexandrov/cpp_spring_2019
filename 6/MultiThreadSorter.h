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
    /* data - имя входного файла
     * output - имя выходного файла
     */
    std::string data, output;
    // кол-во используемых для сортировки потоков
    int threadNum;
    /* memLim - кол-во памяти, выделенной под буфер (в байтах)
     * threadMemLim - кол-во памяти в буфере, относящееся к одному потоку
     * threadUintLim - кол-во uint-ов, которое один поток может хранить в своей части буфера
     */
    size_t memLim, threadMemLim, threadUintLim;
    // умный указатель на буфер
    uint64_t *buf;
    // входной поток
    std::ifstream dataStream;
    // очередь с названиями временных файлов
    std::queue<std::string> outputFiles;
    // вектор потоков
    std::vector<std::thread> threads;
    // переменные, необходимые для синхронизации потоков
    std::atomic<int> finishedStep = 0, finishedSort = 0;
    std::condition_variable cond;
    std::mutex sortFinishM, outQueueM, streamReadM, iterFinishM;

    /* подготовка файлов к соединению (чтение из начального файла + сортировка + сохранение во временные)
     * buffer - указатель на начало части буфера, относящейся к этому потоку
     */
    void prepare(uint64_t * const buffer, const int id) {
        // порядковый номер части файла, отсортированной данным потоком
        int file = 0;
        // пока не дошли до конца файла
        while (dataStream.good()) {
            // блокируем возможность чтения из файла
            std::unique_lock<std::mutex> lock(streamReadM);
            // читаем в часть буфера, относящейся к нашему потоку из начального файла
            dataStream.read(reinterpret_cast<char *>(buffer), threadMemLim);
            // считаем количество прочитанных чисел
            std::streamsize curSize = dataStream.gcount() / sizeof(uint64_t);
            // разблокируем доступ к файлу, чтобы другие потоки могли считывать оттуда, пока мы сортируем уже считанное
            lock.unlock();
            if (curSize != 0) {
                // сортируем эту часть буфера
                std::sort(buffer, buffer + curSize);
                /* формируем уникальноеназвание файла, куда будем сохранять отсортированные данные
                 * id - номер потока
                 * file - номер файла, созданного этим потоком
                 */
                std::string name = std::to_string(0) + '_' + std::to_string(id) + "_" + std::to_string(file) + ".tmp";
                // создаём такой файл
                std::ofstream out(name, std::ios::binary);
                // записываем в него эту часть буфера
                out.write(reinterpret_cast<char*>(buffer), curSize * sizeof(uint64_t));
                // сохраняем имя файла в очередь для последующего объединения
                outputFiles.push(name);
                // увеличиваем счётчик отсортированных этим потоком частей частей
                ++file;
            }
        }
    }

    void merge(const std::string & str1, const std::string & str2, uint64_t * const buf, const int id, const int iter, const int file) {
        // открываем потоки на чтение для объединяемых файлов
        std::ifstream f1(str1, std::ios::binary), f2(str2, std::ios::binary);
        // формируем уникальное название для файла в который будем сохранять результат объединения
        const std::string name = std::to_string(iter) + '_' + std::to_string(id) + '_' + std::to_string(file) + ".tmp";
        // открываем поток на запись в этот файл
        std::ofstream out(name, std::ios::binary);
        /* делим часть буфера нашего потока на 4 части
         * 1 для считывания из f1
         * 1 для считывания из f2
         * 2 для результата их объединения
         */
        const size_t lim = threadUintLim / 4, limM = threadUintLim / 2;
        uint64_t* const bufL = buf, * const bufR = buf + lim, * const bufM = buf + limM;
        // считываем в соответствующие части буфера
        f1.read(reinterpret_cast<char*>(bufL), lim * sizeof(uint64_t));
        // считаем (от слова счёт), сколько считали (считали из файла)
        size_t countFirst = f1.gcount() / sizeof(uint64_t);
        // аналогично
        f2.read(reinterpret_cast<char*>(bufR), lim * sizeof(uint64_t));
        size_t countSecond = f2.gcount() / sizeof(uint64_t);

        size_t currFirst = 0, currSecond = 0, countReady = 0;
        // процесс объединения
        while (!f1.eof() || !f2.eof() || currFirst < countFirst || currSecond < countSecond) {
            //если записали в итоговый файл все числа, считанные из первого файла, то считываем новые (если они есть)
            if (currFirst == countFirst && f1.good()) {
                f1.read(reinterpret_cast<char*>(bufL), lim * sizeof(uint64_t));
                // обновляем кол-во элементов
                countFirst = f1.gcount() / sizeof(uint64_t);
                // обнуляем сдвиг
                currFirst = 0;
            }
            //если записали в итоговый буфер все числа, считанные из второго файла, то считываем новые (если они есть)
            if (currSecond == countSecond && f2.good()) {
                f2.read(reinterpret_cast<char*>(bufR), lim * sizeof(uint64_t));
                // обновляем кол-во элементов
                countSecond = f2.gcount() / sizeof(uint64_t);
                // обнуляем сдвиг
                currSecond = 0;
            }
            // если заполнился итоговый буфер
            if (countReady == limM) {
                // записываем его в итоговый файл
                out.write(reinterpret_cast<char*>(bufM), countReady * sizeof(uint64_t));
                // обнуляем кол-во подготовленных к сохранению элементов
                countReady = 0;
            }
            /* если не дошли до конца считанных из файлов 1 и 2 частей,
             * то выбираем меньшее из двух чисел и записываем в итоговый массив
             * увеличиваем счётчики в итоговом массиве и том, откуда взяли меньшее число
             */
            if (currFirst < countFirst && currSecond < countSecond) {
                if (bufR[currSecond] < bufL[currFirst]) {
                    bufM[countReady] = bufR[currSecond];
                    ++currSecond;
                } else {
                    bufM[countReady] = bufL[currFirst];
                    ++currFirst;
                }
                ++countReady;
            // если первый файл закончился, а второй ещё нет
            } else if (currFirst == countFirst && currSecond < countSecond) {
                bufM[countReady] = bufR[currSecond];
                ++countReady;
                ++currSecond;
            // если второй файл закончился, а первый ещё нет
            } else if (currSecond == countSecond && currFirst < countFirst) {
                bufM[countReady] = bufL[currFirst];
                ++currFirst;
                ++countReady;
            }
        }

        out.write(reinterpret_cast<char*>(bufM), countReady * sizeof(uint64_t));
        std::unique_lock<std::mutex> qlock(outQueueM);
        outputFiles.push(name);
    }

    /* метод, описывающий работу каждого потока в отдельности
     * id - номер потока
     */
    void threadSorter(const int id) {
        // каждый поток работает со своей частью буфера, которая определяется его номером
        uint64_t * const locBuf = buf + id * threadUintLim;
        int iter = 0, file = 0;
        
        prepare(locBuf, id);
        ++iter;
        // блок, отвественный за ожидание потоком того момента, когда все потоки закончат сортировку
        {
            std::unique_lock<std::mutex> lock(iterFinishM);
            ++finishedStep;

            cond.notify_all();
            while (finishedStep < threadNum)
                cond.wait(lock);
        }
        // начинаем объединение файлов с отсортированными числами
        while (outputFiles.size() >= 2) {
            // блокируем доступ к очереди файлов (очереди с названиями файлов)
            std::unique_lock<std::mutex> qLock(outQueueM);
            if (outputFiles.size() >= 2) {
                // вытаскиваем из очереди имя одного файла
                std::string tmp1 = outputFiles.front();
                outputFiles.pop();
                // и второго
                std::string tmp2 = outputFiles.front();
                outputFiles.pop();
                // разблокируем доступ к очереди
                qLock.unlock();
                // соединяем эти 2 файла в один новый
                merge(tmp1, tmp2, locBuf, id, iter, file);
                // удаляем эти файлы, так как мы их уже объединили
                std::remove(tmp1.c_str());
                std::remove(tmp2.c_str());
                // увеличиваем счётчик файлов, полученных при объединении этим потоком для обеспечения уникальности их имён 
                ++file;
            }
        }
        // увеличиваем счётчик потоков, завершивших свою работу
        std::unique_lock<std::mutex> fLock(sortFinishM);
        ++finishedSort;
        // если это последний поток, завершающий работу, то выводим информацию о статусе сортировки
        if (finishedSort == threadNum) {
            if (outputFiles.empty())
                throw std::logic_error("outputFiles is empty!");
            else {
                // переименовываем последний файл
                std::rename(outputFiles.front().c_str(), output.c_str());
                outputFiles.pop();
                std::cout << "Finished, result in file: " << output << std::endl;
            }
        }
    }
public:
    /* конструктор класса
     * inputFilename - имя сортируемого файла
     * outputFilename - имя файла с результатом сортировки
     * threadCount - кол-во используемых потоков
     * memoryLimit - размер буфера (в байтах)
     */
    MultiThreadSorter (std::string inputFilename,std::string outputFilename, int threadCount, size_t memoryLimit) :
        data (inputFilename),
        output (outputFilename),
        threadNum (threadCount),
        memLim (memoryLimit),
        threadMemLim (memoryLimit / threadCount),
        threadUintLim (memoryLimit / (threadCount * sizeof(uint64_t))),
        buf (new uint64_t[memoryLimit / sizeof(uint64_t)]),
        dataStream(data, std::ios::binary)
    {}
    ~MultiThreadSorter () {
        delete[] buf;
    }
    /* метод сортировки */
    void sort () {

        // добавляем потоки в очередь
        for (int i = 0; i < threadNum; ++i)
            threads.emplace_back(&MultiThreadSorter::threadSorter, this, i);
        // запускаем работу потоков
        for (int i = 0; i < threadNum; ++i)
            threads[i].join();
    }
};