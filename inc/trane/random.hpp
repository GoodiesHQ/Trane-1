#ifndef TRANE_RANDOM_HPP
#define TRANE_RANDOM_HPP

#include <random>
#include <array>
#include <algorithm>

namespace trane
{

    template<typename Generator = std::mt19937>
    class Random
    {
    public:
        Random();
        void seed();

        template<typename T = unsigned int>
        T randrange(T lo, T hi);
        typename Generator::result_type gen();

    private:
        bool m_seeded {false};
        std::random_device m_rd;
        Generator m_gen;
    };
}


/*
 * IMPLEMENTATION
 */


template<typename Generator>
trane::Random<Generator>::Random()
{ }


template<typename Generator>
void trane::Random<Generator>::seed()
{
    if(m_seeded)
    {
        return;
    }
    std::array<typename Generator::result_type, Generator::state_size> state;  // store the random seed state with "optimized" 64 bit ints
    std::generate(std::begin(state), std::end(state),
        [this](){
            return m_rd();
        }
    );
    std::seed_seq seq(std::cbegin(state), std::cend(state));
    m_gen.seed(seq);
    m_seeded = true;
}

template<typename Generator>
template<typename T>
T trane::Random<Generator>::randrange(T lo, T hi)
{
    this->seed();
    std::uniform_int_distribution<T> dis(lo, hi);
    return dis(this->m_gen);
}


template<typename Generator>
typename Generator::result_type trane::Random<Generator>::gen()
{
    this->seed();
    return this->m_gen();
}

#endif
