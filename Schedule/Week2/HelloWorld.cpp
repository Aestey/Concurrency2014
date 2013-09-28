/*
 * Display Device Information, output Hello World
 *
 * Taken from http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/introductory-tutorial-to-opencl/
 * "Intro OpenCL Tutorial,  Written by Benedict R. Gaster, AMD Architect, OpenCL
 *
 * Combined with code from Tom Deakin https://github.com/HandsOnOpenCL/Exercises-Solutions/blob/master/Exercises/Exercise01/Cpp/DeviceInfo.cpp
 * Updated by Anthony Estey, 2013
 *
*/

#include <utility>
#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#include <CL/cl.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <vector>

const std::string hw("Hello World\n");

inline void checkErr(cl_int err, const char * name) {
	if (err != CL_SUCCESS) {
		std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

int main(void) {
	cl_int err;
	cl::vector< cl::Platform > platformList;
	
	cl::Platform::get(&platformList);
	checkErr(platformList.size()!=0 ? CL_SUCCESS : -1, "cl::Platform::get");
	std::cerr << "Platform number is: " << platformList.size() << std::endl;
	
	std::string s;
	platformList[0].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &s);
	std::cerr << "Platform is by: " << s << "\n";

	platformList[0].getInfo((cl_platform_info)CL_PLATFORM_VERSION, &s);
	std::cerr << "Version is: " << s << "\n";	

	cl_context_properties cprops[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};cl::Context context(CL_DEVICE_TYPE_CPU,cprops,NULL,NULL,&err);
	checkErr(err, "Conext::Context()"); 

	char * outH = new char[hw.length()+1];
	cl::Buffer outCL(context,CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,hw.length()+1,outH,&err);
	checkErr(err, "Buffer::Buffer()");
 
	cl::vector<cl::Device> devices;
	platformList[0].getDevices((cl_platform_info)CL_DEVICE_TYPE_ALL, &devices);
    std::cout << "Number of devices: " << devices.size() << std::endl;

	for (int devNum=0; devNum<devices.size(); devNum++) {
		cl::Device dev = devices[devNum];
        std::cout << "-------------------------" << std::endl;

        dev.getInfo(CL_DEVICE_NAME, &s);
        std::cout << "Name: " << s << std::endl;

        dev.getInfo(CL_DEVICE_OPENCL_C_VERSION, &s);
        std::cout << "Version: " << s << std::endl;

        int i;
        dev.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &i);
        std::cout << "Max. Compute Units: " << i << std::endl;

        size_t size;
        dev.getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &size);
        std::cout << "Local Memory Size: " << size/1024 << " KB" << std::endl;

        dev.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &size);
        std::cout << "Global Memory Size: " << size/(1024*1024) << " MB" << std::endl;

        dev.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &size);
        std::cout << "Max Alloc Size: " << size/(1024*1024) << " MB" << std::endl;

        dev.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &size);
        std::cout << "Max Work-group Size: " << size << std::endl;

        std::cout << "-------------------------" << std::endl;
	}

	devices = context.getInfo<CL_CONTEXT_DEVICES>();
	checkErr(
	devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");


	std::ifstream file("HelloWorld_Kernel.cl");
	checkErr(file.is_open() ? CL_SUCCESS:-1, "lesson1_kernel.cl");std::string prog(
	std::istreambuf_iterator<char>(file),
	(std::istreambuf_iterator<char>()));cl::Program::Sources source(1,
	std::make_pair(prog.c_str(), prog.length()+1));cl::Program program(context, source);
	err = program.build(devices,"");
	checkErr(err, "Program::build()");


	cl::Kernel kernel(program, "hello", &err);
	checkErr(err, "Kernel::Kernel()");err = kernel.setArg(0, outCL);
	checkErr(err, "Kernel::setArg()");


	cl::CommandQueue queue(context, devices[0], 0, &err);
	checkErr(err, "CommandQueue::CommandQueue()");cl::Event event;
	err = queue.enqueueNDRangeKernel(
	kernel,
	cl::NullRange,
	cl::NDRange(hw.length()+1),
	cl::NDRange(1, 1),
	NULL,
	&event);
	checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");


	event.wait();
	err = queue.enqueueReadBuffer(
	outCL,
	CL_TRUE,
	0,
	hw.length()+1,
	outH);
	checkErr(err, "ComamndQueue::enqueueReadBuffer()");
	std::cout << outH;
	return EXIT_SUCCESS;
}
