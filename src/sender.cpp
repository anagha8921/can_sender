#include <rclcpp/rclcpp.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <random>
#include <bit>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
// #include <linux/can.h>
// #include <linux/can/raw.h>
#include <cstdio>
#include <cstring>
#include "std_msgs/msg/string.hpp"
#include "dbc/dbc_parser.h"
#include "v2c/v2c_transcoder.h"
#include "aox_can_msgs/msg/set_conf1.hpp"

std::string read_file(const std::string &dbc_path)
{
    std::ifstream dbc_content(dbc_path);
    std::ostringstream ss;
    ss << dbc_content.rdbuf();
    return ss.str();
}
// can_frame convertCh1ConfigInfoToCANFrame(std::make_shared<aox_can_msgs::msg::SetConf1>() conf1_set_msg)
// {
//     can_frame frame;
//     double ch4_setpoint = conf1_set_msg->conf1_ch4_setpoint;
//     double ch3_setpoint = conf1_set_msg->conf1_ch3_setpoint;
//     double ch2_setpoint = conf1_set_msg->conf1_ch2_setpoint;
//     double ch1_setpoint = conf1_set_msg->conf1_ch1_setpoint;

//     frame.can_id = 6;  // Message ID
//     frame.can_dlc = 8; // Number of data bytes
//     frame.can_extended = false;
//     frame.can_rtr = false;

//     // Convert ch4_setpoint to two bytes
//     int16_t ch4_setpoint_bytes = static_cast<int16_t>((ch4_setpoint + 800) / 0.05);
//     frame.data[0] = ch4_setpoint_bytes & 0xFF;
//     frame.data[1] = (ch4_setpoint_bytes >> 8) & 0xFF;

//     // Convert ch3_setpoint to two bytes
//     int16_t ch3_setpoint_bytes = static_cast<int16_t>(ch3_setpoint / 0.5);
//     frame.data[2] = ch3_setpoint_bytes & 0xFF;
//     frame.data[3] = (ch3_setpoint_bytes >> 8) & 0xFF;

//     // Convert ch2_setpoint to two bytes
//     int16_t ch2_setpoint_bytes = static_cast<int16_t>(ch2_setpoint / 0.5);
//     frame.data[4] = ch2_setpoint_bytes & 0xFF;
//     frame.data[5] = (ch2_setpoint_bytes >> 8) & 0xFF;

//     // Convert ch1_setpoint to two bytes
//     int16_t ch1_setpoint_bytes = static_cast<int16_t>(ch1_setpoint / 0.5);
//     frame.data[6] = ch1_setpoint_bytes & 0xFF;
//     frame.data[7] = (ch1_setpoint_bytes >> 8) & 0xFF;

//     return frame;
// }
class DBCParsingNode : public rclcpp::Node
{
public:
    DBCParsingNode() : Node("can_sender")
    {
        setupSocket();
        parseDBCFrames();
    }

private:
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    can::v2c_transcoder transcoder;
    int32_t frameCounter = 0;

    int setupSocket()
    {
        // Create a socket
        s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (s == -1)
        {
            perror("Socket creation error");
            return EXIT_FAILURE;
        }

        // std::strcpy(ifr.ifr_name, "can_qemu");
        std::strcpy(ifr.ifr_name, "vcan0");

        // Find the interface index using ioctl
        if (ioctl(s, SIOCGIFINDEX, &ifr) == -1)
        {
            perror("ioctl error");
            close(s);
            return EXIT_FAILURE;
        }

        // Set the address parameters
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // Bind the socket to the CAN interface
        if (bind(s, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1)
        {
            perror("Bind error");
            close(s);
            return EXIT_FAILURE;
        }
    }

    void parseDBCFrames()
    {

        auto start = std::chrono::system_clock::now();
        // bool parsed = can::parse_dbc(read_file("/home/devuser/AOX/ROS2/src/can-utils/example/example.dbc"), std::ref(transcoder));
        bool parsed = can::parse_dbc(read_file("/home/devuser/AOX/ROS2/mspc_set_msgs.dbc"), std::ref(transcoder));
        // bool parsed = can::parse_dbc(read_file("/home/devuser/AOX/ROS2/mspc_fdbk_msgs.dbc"), std::ref(transcoder));
        auto end = std::chrono::system_clock::now();
        std::cout << "Parsed DBC in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
        if (!parsed)
            std::cout << "Parsing not successful" << std::endl;
        else
            std::cout << "Parsing successful!!" << std::endl;
        // while (rclcpp::ok())
        // {
        SendCANFrames();
        // }
    }
    void SendCANFrames()
    {
        can_frame frame;
        // Populate the BatteryInfo message with data
        auto conf1_set_msg = std::make_shared<aox_can_msgs::msg::SetConf1>();
        conf1_set_msg->conf1_ch4_setpoint = 1;
        conf1_set_msg->conf1_ch3_setpoint = 4;
        conf1_set_msg->conf1_ch2_setpoint = 789;
        conf1_set_msg->conf1_ch1_setpoint = 5;
        // // Convert BatteryInfo message to CAN frame using the converter
        // can_frame can_frame = convertCh1ConfigInfoToCANFrame(conf1_set_msg);
        double ch4_setpoint = conf1_set_msg->conf1_ch4_setpoint;
        double ch3_setpoint = conf1_set_msg->conf1_ch3_setpoint;
        double ch2_setpoint = conf1_set_msg->conf1_ch2_setpoint;
        double ch1_setpoint = conf1_set_msg->conf1_ch1_setpoint;

        frame.can_id = 133; // Message ID
        frame.can_dlc = 8;  // Number of data bytes

        // Convert ch4_setpoint to two bytes
        int16_t ch4_setpoint_bytes = static_cast<int16_t>((ch4_setpoint));
        frame.data[0] = ch4_setpoint_bytes & 0xFF;
        frame.data[1] = (ch4_setpoint_bytes >> 8) & 0xFF;

        // Convert ch3_setpoint to two bytes
        int16_t ch3_setpoint_bytes = static_cast<int16_t>(ch3_setpoint);
        frame.data[2] = ch3_setpoint_bytes & 0xFF;
        frame.data[3] = (ch3_setpoint_bytes >> 8) & 0xFF;

        // Convert ch2_setpoint to two bytes
        int16_t ch2_setpoint_bytes = static_cast<int16_t>(ch2_setpoint);
        frame.data[4] = ch2_setpoint_bytes & 0xFF;
        frame.data[5] = (ch2_setpoint_bytes >> 8) & 0xFF;

        // Convert ch1_setpoint to two bytes
        int16_t ch1_setpoint_bytes = static_cast<int16_t>(ch1_setpoint);
        frame.data[6] = ch1_setpoint_bytes & 0xFF;
        frame.data[7] = (ch1_setpoint_bytes >> 8) & 0xFF;
        // Send the CAN frame
        ssize_t bytesSent = write(s, &frame, sizeof(struct can_frame));
        std::cout << "Send CAN frame to define set-points for configuration A:" << std::endl;
        std::cout << "  ID: 0x" << std::hex << frame.can_id << std::dec << std::endl;
        std::cout << "  DLC: " << static_cast<int>(frame.can_dlc) << std::endl;
        std::cout << "  Data: ";
        for (int i = 0; i < frame.can_dlc; ++i)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(frame.data[i]) << " ";
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(4));
        ReceiveCANFrames(frame);
    }
    void ReceiveCANFrames(can_frame send_frame)
    {
        bool parsed = can::parse_dbc(read_file("/home/devuser/AOX/ROS2/mspc_fdbk_msgs.dbc"), std::ref(transcoder));
        struct can_frame frame;
        ssize_t nbytes = read(s, &frame, sizeof(struct can_frame));

        if (nbytes < 0)
        {
            perror("Read error");
        }
        else if (nbytes < sizeof(struct can_frame))
        {
            std::cerr << "Incomplete CAN frame received" << std::endl;
        }
        else
        {
            processCANFrame(frame, send_frame);
        }
    }
    void processCANFrame(const struct can_frame &frame, const struct can_frame &send_frame)
    {
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_ = this->create_publisher<std_msgs::msg::String>("Config1_SetPoints_1", 10);
        std::cout << "Received CAN frame:" << std::endl;
        std::cout << "  ID: 0x" << std::hex << frame.can_id << std::dec << std::endl;
        std::cout << "  DLC: " << static_cast<int>(frame.can_dlc) << std::endl;
        std::cout << "  Data: ";
        for (int i = 0; i < frame.can_dlc; ++i)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(frame.data[i]) << " ";
        }
        std::cout << std::endl;

        frameCounter++;
        std::cout << "Transcoding abt to start!" << std::endl;

        auto fp = transcoder.transcode(std::chrono::system_clock::now(), frame);

        std::cout << "Signal values of received message:" << std::endl;
        auto frame_data = std::bit_cast<int64_t>(frame.data);
        // auto t = duration_cast<milliseconds>(ts.time_since_epoch()).count() / 1000.0;

        auto msg = transcoder.find_message(frame.can_id);
        for (const auto &sig : msg->signals(frame_data))
            std::cout << "  " << sig.name() << ": " << (int64_t)sig.decode(frame_data) << std::endl;

        if ((frame.data[0] == send_frame.data[0]) && (frame.data[1] == send_frame.data[1]) && (frame.data[2] == send_frame.data[2]) && (frame.data[3] == send_frame.data[3]))
        {
            std::cout << "set-points for the configuration A were successfully defined!!!" << std::endl;
            auto message = std_msgs::msg::String();
            message.data = "Configuration A  set points defined!";
            RCLCPP_INFO(this->get_logger(), "Publishing: %s", message.data.c_str());
            publisher_->publish(message);
        }
        if (!fp.empty())
            frameCounter = 0;
        // print_frames(fp, transcoder, frame_counter);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DBCParsingNode>());
    rclcpp::shutdown();
    return 0;
}