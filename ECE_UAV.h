//multithreading and header files
#pragma once
#include<atomic>
#include<mutex>
#include<thread>
#include<string>

class ECE_UAV
{
public:
	void start();
	void stop();
	void getPosition(double* inPos)
	{
		mtx.lock();
		memcpy(inPos, m_Position, 3 * sizeof(double));
		mtx.unlock();
	}
	void position(double* inPos)
	{
		mtx.lock();
		memcpy(m_Position, inPos, 3 * sizeof(double));
		mtx.unlock();
	}
	friend void threadFunction(ECE_UAV* pUAV);
private:
	std::atomic<bool> m_bStop = false;
	double m_mass = 1.0;
	double m_Position[3] = {0,0,0};
	double m_Velocity[3] = {0,0,0};
	double m_Acceleration[3] = {2,2,2};
	double number;
	std::thread m_KinematicsThread;
	std::mutex mtx;
};