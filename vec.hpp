#ifndef VEC_HPP
#define VEC_HPP

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>

template <typename T>
class Vec {
    private:
        T* data;
        uint32_t used;
        uint32_t cap;

    public:
        Vec(){
            data = new T[10];
            used = 0;
            cap = 10;
        }
        Vec(const Vec& other) {
            cap = other.cap;
            used = other.used;
            data = new T[cap]; memcpy(data, other.data, used * sizeof(T)); }

        Vec(int64_t size){
            data = new T[size];
            used = 0;
            cap = size;
        }

        ~Vec(){
            delete[] data;
        }

        Vec operator+(const Vec& vec) const {
            Vec new_vec((vec.used + used) * 2);
            try {
                memcpy(new_vec.data, data, used * sizeof(T));
                memcpy(&new_vec.data[used], vec.data, vec.used * sizeof(T));
                new_vec.used = vec.used + used;
            } catch (...) {
                delete[] new_vec.data;
                throw;
            }
            return new_vec;
        }

        Vec<T> operator+(Vec<T> vec){
            Vec<T> new_vec((vec.used + used) * 2);
            memcpy(new_vec.data, data, used * sizeof(T));
            memcpy(&new_vec.data[used], vec.data, vec.used * sizeof(T));
            new_vec.used = vec.used + used;
            return std::move(new_vec);
        }

        T& operator[](uint64_t index){
            return data[index];
        }

        const T& operator[](uint64_t index) const {
            return data[index];
        }

        void append(T value){
            if(used < cap){
                data[used++] = value;
            } else{
                data = (T*)realloc(data, (cap * sizeof(T) * 2));
                data[used++] = value;
            }
        }

        void swap_remove(size_t index){
            data[index] = data[--used];
        }

        void remove(size_t index){
            std::memcpy(&data[index], &data[++index], (used-- - index) * sizeof(T));
        }

        void insert(size_t index, T value){
            memcpy(&data[index + 1], &data[index], (used++ - index) * sizeof(T));
            data[index] = value;
        }

        T pop_front(){
            T result = data[0];
            memcpy(&data[0], &data[1], --used * sizeof(T));
            return result;
        }

        T get_last(){
            return data[used - 1];
        }

        void print(){
            for(uint64_t i = 0; i < used; ++i){
                std::cout << "index " << i << ": " << data[i] << "\n";
            }
        }

        const uint64_t len() const{
            return used;
        }

        uint64_t len(){
            return used;
        }
};

#endif
