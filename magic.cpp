#include <iostream>

struct PasDEntier
{
    PasDEntier(double) {}
    PasDEntier(int) = delete;
};

struct SeumentDesDoubles
{
    SeumentDesDoubles(double) {}
    template<typename T>
    SeumentDesDoubles(T) = delete;
};

int main()
{
    {
        //PasDEntier p = 5; // error: conversion function from 'int' to 'PasDEntier' invokes a deleted function
        [[maybe_unused]]PasDEntier q = 5.3;
        [[maybe_unused]]PasDEntier r = 1.9f;
    }
    {
        //SeumentDesDoubles p = 5; // error: conversion function from 'int' to 'PasDEntier' invokes a deleted function
        [[maybe_unused]]SeumentDesDoubles q = 5.3;
        //SeumentDesDoubles r = 1.9f; // error: conversion function from 'float' to 'SeumentDesDoubles' invokes a deleted function
    }
}