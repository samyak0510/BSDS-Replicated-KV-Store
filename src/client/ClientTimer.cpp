#include <iomanip>
#include <iostream>

#include "ClientTimer.h"

ClientTimer::ClientTimer() {
	sum = duration<double, std::micro>(0);
	max = duration<double, std::micro>(0);
	min = duration<double, std::micro>(9999999999.9f);
	op_count = 0;
}

void ClientTimer::Start() {
	start_time = high_resolution_clock::now();
}

void ClientTimer::End() {
	auto end_time = high_resolution_clock::now();
	elapsed_time = (end_time - start_time);
}

void ClientTimer::EndAndMerge() {
	End();
	op_count++;
	sum += elapsed_time;
	if (elapsed_time < min) {
		min = elapsed_time;
	}
	if (elapsed_time > max) {
		max = elapsed_time;
	}
}

void ClientTimer::Merge(ClientTimer timer) {
	sum += timer.sum;
	op_count += timer.op_count;
	if (timer.min < min) {
		min = timer.min;
	}
	if (timer.max > max) {
		max = timer.max;
	}	
}

void ClientTimer::PrintStats() {
  std::cout << std::fixed << std::setprecision(3);

  if (op_count == 0) {
    std::cout << "0\t0\t0\t0" << std::endl;
    return;
  }

  const double ops = static_cast<double>(op_count);
  const double total_us = sum.count();                 
  const double avg_us   = total_us / ops;              
  const double min_us   = min.count();
  const double max_us   = max.count();
  const double tput     = (total_us > 0.0) ? (ops / total_us * 1e6) : 0.0; 

  std::cout << avg_us << "\t" << min_us << "\t" << max_us << "\t" << tput << std::endl;
}


