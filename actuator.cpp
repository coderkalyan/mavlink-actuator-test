#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <future>
#include <memory>

using namespace mavsdk;

void send_actuator(std::shared_ptr<MavlinkPassthrough> mavlink_passthrough,
        float value1, float value2, float value3);

int main(int argc, char **argv)
{
    Mavsdk dc;
    std::string connection_url;
    ConnectionResult connection_result;
    float value1, value2, value3;

    if (argc == 5) {
        connection_url = argv[1];
        connection_result = dc.add_any_connection(connection_url);
        value1 = std::stof(argv[2]);
        value2 = std::stof(argv[3]);
        value3 = std::stof(argv[4]);
    } 

    if (connection_result != ConnectionResult::Success) {
        std::cout << "Connection failed: " << connection_result << std::endl;
        return 1;
    }

    std::promise<void> prom;
    std::future<void> fut = prom.get_future();
    std::cout << "Waiting to discover system..." << std::endl;
    dc.register_on_discover([&prom](uint64_t /* uuid*/) { prom.set_value(); });

    if (fut.wait_for(std::chrono::seconds(2)) != std::future_status::ready) {
        std::cout << "No device found, exiting." << std::endl;
        return 1;
    }

    System& system = dc.system();

    auto mavlink_passthrough = std::make_shared<MavlinkPassthrough>(system);

    send_actuator(mavlink_passthrough, value1, value2, value3);

    return 0;
}

void send_actuator(std::shared_ptr<MavlinkPassthrough> mavlink_passthrough,
        float value1, float value2, float value3)
{
    std::cout << "Sending message" << std::endl;
    mavlink_message_t message;
    mavlink_msg_command_long_pack(
            mavlink_passthrough->get_our_sysid(),
            mavlink_passthrough->get_our_compid(),
            &message,
            mavlink_passthrough->get_target_sysid(),
            mavlink_passthrough->get_target_compid(),
            MAV_CMD_DO_SET_ACTUATOR,
            0,
            value1, value2, value3,
            NAN, NAN, NAN, 0);
    mavlink_passthrough->send_message(message);
    std::cout << "Sent message" << std::endl;
}
