#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

namespace ylems
{
    namespace elements
    {
        template<typename T, size_t N>
        class CycleBuffer
        {
        public:
            template<typename U>
            void push_back(U&& u)
            {
                if(depth_ < N)
                    ++depth_;
                inc(l_);
                data_[l_] = FWD(u);
            }

            void clear()
            {
                depth_ = 0;
                l_     = N-1;
            }

            size_t size() const { return depth_; }
            static constexpr size_t capacity() { return N; }

            T const& back(size_t i = 0) const
            {
                assert(i < N);
                return data_[(N + l_ - i)%N];
            }

        private:
            static void inc(size_t& i)
            {
                if(++i == N)
                    i = 0;
            }

            T       data_[N] = {};
            size_t  l_       = N-1;
            size_t  depth_   = 0;
        };

    }
}