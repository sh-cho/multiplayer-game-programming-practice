#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory>
#include <cstdint>
#include <cstring>


class SocketAddress {
public:
	SocketAddress(uint32_t in_addr, uint16_t in_port) {
		get_as_sock_addr_in()->sin_family = AF_INET;
		get_as_sock_addr_in()->sin_addr.s_addr = htonl(in_addr);
		get_as_sock_addr_in()->sin_port = htons(in_port);
	}

	SocketAddress(const sockaddr& in_sock_addr) {
		memcpy(&_sock_addr, &in_sock_addr, sizeof(sockaddr));
	}

	static SocketAddressPtr create_ipv4_from_string(const std::string& in_string) {
		// example format: "google.com:80"
		
		auto pos = in_string.find_last_of(':');
		std::string host, service;
		if (pos != std::string::npos) {
			host = in_string.substr(0, pos);
			service = in_string.substr(pos + 1);
		} else {
			host = in_string;
			service = "0";
		}

		addrinfo hint;
		memset(&hint, 0, sizeof(hint));
		hint.ai_family = AF_INET;

		addrinfo *result;
		int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &result);
		addrinfo *init_result = result;

		if (error != 0 && result != nullptr) {
			freeaddrinfo(init_result);
			return nullptr;
		}

		while (!result->ai_addr && result->ai_next) {
			result = result->ai_next;
		}
		if (!result->ai_addr) {
			freeaddrinfo(init_result);
			return nullptr;
		}

		SocketAddressPtr ret = std::make_shared<SocketAddress>(*result->ai_addr);
		freeaddrinfo(init_result);
		return ret;
	}

	size_t get_size() const {
		return sizeof(sockaddr);
	}

private:
	sockaddr _sock_addr;
	sockaddr_in *get_as_sock_addr_in() {
		return reinterpret_cast<sockaddr_in *>(&_sock_addr);
	}
};


using SocketAddressPtr = std::shared_ptr<SocketAddress>;
