//include header files
#include "ECE_UAV.h"
#include<chrono>
#include<cmath>

double time = 0, totalTime = 0, k = 0.005;
double scaleFactor = 0;
void threadFunction(ECE_UAV* pUAV)
{
	double xSphere(0.0f), ySphere(20.0f), zSphere(0.0f); //sphere position
	scaleFactor = 1000.0; //time will be scaled
	bool collide = false;
	std::this_thread::sleep_for(std::chrono::seconds(5));
	do
	{
		totalTime = ((time / 100) / scaleFactor);
		pUAV->getPosition(pUAV->m_Position);//uav position update

		//normalizing velocity from initial velocity
		double velocity = sqrt(pow(pUAV->m_Velocity[0], 2) + pow(pUAV->m_Velocity[1], 2) + pow(pUAV->m_Velocity[2], 2));
		double xVelocity = pUAV->m_Velocity[0] / velocity;
		double yVelocity = pUAV->m_Velocity[1] / velocity;
		double zVelocity = pUAV->m_Velocity[2] / velocity;

		//distance to centre of sphere
		double distX = 0.0 - pUAV->m_Position[0];
		double distY = 20.0 - pUAV->m_Position[1];
		double distZ = 0.0 - pUAV->m_Position[2];
		double distSphereCenter = sqrt(pow(distX, 2) + pow(distY, 2) + pow(distZ, 2));

		//scaled image distance computation
		double ang = (pUAV->number - 1) * 20;
		double x = 80 * sin(ang * (3.14159f / 180)) - pUAV->m_Position[0];
		double y = 80 * cos(ang * (3.14159f / 180)) - pUAV->m_Position[1];
		double z = 0.0 - pUAV->m_Position[2];
		double dist = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

		//normal part of force between sphere and uav object
		double xNormal = x / dist;
		double yNormal = y / dist;
		double zNormal = z / dist;
		double normalFunc = -k * (10 - distSphereCenter);

		//updated uav velocity
		double xVelocityNew = pUAV->m_Velocity[0] + pUAV->m_Acceleration[0] * totalTime;
		double yVelocityNew = pUAV->m_Velocity[1] + pUAV->m_Acceleration[1] * totalTime;
		double zVelocityNew = pUAV->m_Velocity[2] + pUAV->m_Acceleration[2] * totalTime;

		pUAV->m_Velocity[0] = xVelocityNew;
		pUAV->m_Velocity[1] = yVelocityNew;
		pUAV->m_Velocity[2] = zVelocityNew;

		//updated distance
		double xDistanceNew = pUAV->m_Position[0] - pUAV->m_Velocity[0] + 0.5 * pUAV->m_Acceleration[0] * (pow(totalTime, 2));
		double yDistanceNew = pUAV->m_Position[1] - pUAV->m_Velocity[1] + 0.5 * pUAV->m_Acceleration[1] * (pow(totalTime, 2));
		double zDistanceNew = pUAV->m_Position[2] - pUAV->m_Velocity[2] + 0.5 * pUAV->m_Acceleration[2] * (pow(totalTime, 2));

		pUAV->m_Position[0] = xDistanceNew;
		pUAV->m_Position[1] = yDistanceNew;
		pUAV->m_Position[2] = zDistanceNew;

		/*for (int i = 0; i < 15; i++) //checking for collision
		{
			for (int j = i; j < 15; j++)
			{
				if (m_Position[i] == m_Position[j])
				{
					collide = true;
				}
			}
		}*/

		pUAV->position(pUAV->m_Position);
		time++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (!pUAV->m_bStop);
}
void ECE_UAV::start()
{
	m_KinematicsThread = std::thread(threadFunction, this);
}
void ECE_UAV::stop()
{
	m_bStop = true;
	if (m_KinematicsThread.joinable())
	{
		m_KinematicsThread.join();
	}
}