#pragma once

class WELL512 {
private:
    unsigned int state[16];
    unsigned int index;
    void init(unsigned int seed);
public:
    WELL512();
    WELL512(unsigned int seed);
    unsigned long rand();
    unsigned long rand(unsigned long max);
    float randf();
};