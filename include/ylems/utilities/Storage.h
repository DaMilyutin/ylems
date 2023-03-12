#pragma once
#include <limits>
#include <assert.h>
#include <vector>
#include <array>

namespace ylems
{
    namespace elements
    {
        template<typename Under>
        class Storage;

        template<typename T>
        class Storage<std::vector<T>>
        {
        public:
            Storage() = default;

            template<typename U>
            void push_back(U&& u)
            {
                under_.push_back(FWD(u));
            }

            bool full() const { return false; }

            auto begin() const { return under_.begin(); }
            auto end()   const { return under_.end();   }

            T const& back() const { return under_.back(); }

        private:
            std::vector<T> under_;
        };

        template<typename T, size_t N>
        class Storage<std::array<T, N>>
        {
        public:
            template<typename U>
            void push_back(U&& u)
            {
                assert(size_ < N);
                if(size_ == N)
                    return;
                under_[size_++] = FWD(u);
            }

            bool full() const { return size_ == N; }

            auto begin() const { return under_.begin(); }
            auto end()   const { return under_.begin() + size_; }

            T const& back() const { return under_[size_-1]; }

        private:
            size_t           size_ = 0;
            std::array<T, N> under_;
        };



    }
}