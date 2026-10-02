#include "UDPEmitter.h"
#include "BBData.h"
#include <Log/Log.h>

DEFINE_LOG(UDPEmitter)

UDPEmitter::UDPEmitter() noexcept{
	socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(socket == -1){
		CMF_LOG(UDPEmitter, Error, "Can't create socket, errno=%d: %s", errno, strerror(errno));
		return;
	}

	dest.sin_family = AF_INET;
	dest.sin_port = htons(UDPPort);
	inet_pton(AF_INET, ControllerIP, &dest.sin_addr);
}

UDPEmitter::~UDPEmitter() noexcept{
	close(socket);
}

bool UDPEmitter::write(const std::vector<uint8_t>& buffer) noexcept{
	return write(buffer.data(), buffer.size());
}

bool UDPEmitter::write(const uint8_t* buffer, size_t count) noexcept {
    if(socket == -1){
        CMF_LOG(UDPEmitter, Warning, "Write, but socket not set-up");
        return false;
    }

    if(buffer == nullptr || count == 0) {
        return true;
    }

    size_t total = 0;
    while(total < count){
        // The std::min needs to be there cause posix is stupid and if you send more than the max possible, it will not send it, but will return like it has sent a successful amount (fucking idiots)
        const int now = ::sendto(socket, buffer + total, std::min(count - total, (size_t)CONFIG_TCP_MSS), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));

        if(now == 0){
            return false;
        }

        if(now < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                vTaskDelay(1);
                continue;
            }

            return false;
        }

        total += now;
    }

    return true;
}
