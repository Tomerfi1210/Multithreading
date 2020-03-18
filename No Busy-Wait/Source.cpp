#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <memory>


class Sensor;

class Sensor
{
public:
	Sensor() : empty1(true), empty2(true), newest_data(0) { ; }

	void onReceiveMsg(uint8_t msg[], const unsigned int msgSize)
	{
		//short operation with _msg
		if (msgSize < MSG_SIZE) {
			availBuffer(msg, msgSize);
		}
	}

	void handleMsg()
	{
		//long operation with _msg

		if (!empty2 && newest_data == 2)//if there is new data in buffer 2.
		{
			//operation with _msg.
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); //mock up for long action
			empty2 = true;
		}

		if (!empty1 && newest_data == 1)//if there is new data in buffer 1.
		{
			//operation with _msg2.
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); //mock up for long action	
			empty1 = true;
		}
	}

private:
	void availBuffer(uint8_t msg[], const unsigned int msgSize);//enter data to available buffer.
	void copyBuffer(uint8_t dest[], uint8_t src[],bool&, int, const unsigned int msgSize);//func to copy one buffer to anoter, update that he is loaded and the newst.
	static const unsigned int MSG_SIZE = 50;
	uint8_t _msg[MSG_SIZE];
	uint8_t _msg_2[MSG_SIZE];
	bool empty1;//to determine if _msg is empty.
	bool empty2;//to determine if _msg_2 is empty.
	int newest_data;//variable to know which buffer is the newst if both are full.
};

//check which buffer is availble and enter data to him.
void Sensor::availBuffer(uint8_t msg[], const unsigned int msgSize)
{
	if (empty1)//if buffer1 is empty.
		copyBuffer(_msg, msg, empty1, 1, msgSize);

	else if (empty2)//if buffer2 is empty.
		copyBuffer(_msg_2, msg, empty2, 2, msgSize);

	//when both buffers are still full
	else {
		//if we are currently handling the first
		if (newest_data == 1)
			copyBuffer(_msg_2, msg, empty2, 2, msgSize);//use the second buffer

		//if we are currently handling the second buffer
		else
			copyBuffer(_msg, msg, empty1, 1, msgSize);//insert into the first
	}
}

//copy the dest buffer from src and update fields.
void Sensor::copyBuffer(uint8_t dest[], uint8_t src[], bool& empty, int newest_buf, const unsigned int msgSize)
{
	std::memcpy(dest, src, msgSize);
	empty = false;
	newest_data = newest_buf;
}

// ################### DO NOT CHANGE CODE BELOW ################### //

void interrupt(std::shared_ptr<Sensor> pSensor) {
	unsigned int counter = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	start = std::chrono::high_resolution_clock::now();

	static const int WRITER_THREAD_CYCLE_MICRO_SECONDS = 20000;
	static const unsigned int MSG_SIZE = 50;
	uint8_t msg[50];

	while (true) {
		end = std::chrono::high_resolution_clock::now();


		std::chrono::microseconds elapsedmicro = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		int randomNoise = (std::rand() % 1000) * ((std::rand() % 2) * -1); //add random +- 1000 microseconds of noise

		if (elapsedmicro.count() >= (WRITER_THREAD_CYCLE_MICRO_SECONDS + randomNoise)) {

			msg[0] = counter;
			for (unsigned int i = 1; i < MSG_SIZE; i++) {
				msg[i] = std::rand() % 10;
			}

			pSensor->onReceiveMsg(msg, MSG_SIZE - 1);

			counter++;
			start = std::chrono::high_resolution_clock::now();

		}
	}
}

void reader(std::shared_ptr<Sensor> pSensor) {
	unsigned int counter = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	start = std::chrono::high_resolution_clock::now();

	static const int READER_THREAD_CYCLE_MICRO_SECONDS = 27778;

	while (true) {
		end = std::chrono::high_resolution_clock::now();


		std::chrono::microseconds elapsedmicro = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		if (elapsedmicro.count() >= READER_THREAD_CYCLE_MICRO_SECONDS) {
			counter++;
			pSensor->handleMsg();
			start = std::chrono::high_resolution_clock::now();
		}

	}

}

int main() {
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	std::shared_ptr<Sensor> pSensor = std::make_shared<Sensor>();

	//Launch threads
	std::thread t1(interrupt, pSensor);
	std::thread t2(reader, pSensor);

	//Join the thread with the main thread
	t1.join();
	t2.join();

	return 0;
}

