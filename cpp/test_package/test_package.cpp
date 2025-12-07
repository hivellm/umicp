#include <iostream>
#include "umicp/envelope.hpp"

int main() {
    try {
        umicp::Envelope envelope;
        envelope.setFrom("test");
        envelope.setTo("receiver");
        envelope.setOperation(umicp::OperationType::DATA);
        
        std::cout << "UMICP C++ library test successful!" << std::endl;
        std::cout << "Version: 0.1.1" << std::endl;
        std::cout << "Envelope created: " << envelope.getFrom() << " -> " << envelope.getTo() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

