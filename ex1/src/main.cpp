//
// Created by matta on 3/30/2020.
//

#include <iostream>
#include "osm.h"


int main()
{
    auto iterations = static_cast<unsigned int>(1e8);
    double add_avg = osm_operation_time(iterations);
    double func_avg = osm_function_time(iterations);
    double sys_avg = osm_syscall_time(iterations);

    std::cout<<add_avg<<","<<func_avg<<","<<sys_avg<<std::endl;
}
