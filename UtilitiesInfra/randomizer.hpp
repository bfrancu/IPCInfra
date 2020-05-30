#ifndef RANDOMIZER_H
#define RANDOMIZER_H
#include <random>
#include <algorithm>

namespace infra
{

template<typename IntType>
IntType getRandomNumberInRange(IntType lower, IntType upper)
{
    IntType result{0};
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<IntType> distr(lower, upper);
    result = distr(generator);
    return result;
}

} //infra
#endif // RANDOMIZER_H
