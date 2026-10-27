#define _GNU_SOURCE

#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <ackermann_msgs/AckermannDriveStamped.h>

ros::Publisher drive_publisher; // publisher 

ros::Subscriber keyboard_control_subscriber; // subscriber
ros::Subscriber follow_the_gap_subscriber; // subscriber

std::string current_mode = "k"; // selection of mode initial

// Callback for the AEB
void aebCallback(const ackermann_msgs::AckermannDriveStamped::ConstPtr& d_sig) {
     drive_publisher.publish(*d_sig);
     current_mode = "k";
}

// Callback for the keyboard control
void keyboardControlCallback(const ackermann_msgs::AckermannDriveStamped::ConstPtr& d_sig) {
    if (current_mode == "k") {
        drive_publisher.publish(*d_sig);
    }
}

// Callback for the follow-the-gap control
void followTheGapCallback(const ackermann_msgs::AckermannDriveStamped::ConstPtr& d_sig) {
    if (current_mode == "f") {
        drive_publisher.publish(*d_sig);
    }

}

// Mode switching

void mode_selection(const std_msgs::String::ConstPtr& msg) {
    if (msg->data != "f") {
        ROS_INFO("Switching to Keyboard Control");  // Switching to keyboard mode
        current_mode = "k";
    } else {
        ROS_INFO("Switching to Follow-the-Gap Control");  // Switching to follow-the-gap mode
        current_mode = "f";
    }
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "muxmerv");
    ros::NodeHandle node;

   // Advertise the drive topic
    drive_publisher = node.advertise<ackermann_msgs::AckermannDriveStamped>("/drive", 10); // advertise the drive topic
   // Subscribe to the mode and control topics
    ros::Subscriber mode_sub = node.subscribe("/mode", 10, mode_selection); // subscribe to mode selection
    
    keyboard_control_subscriber = node.subscribe("/keyboard_control", 1, keyboardControlCallback);  // subscribe to keyboard
    follow_the_gap_subscriber = node.subscribe("/gap_follow", 10, followTheGapCallback); // subscribe to gap_follow node

    ros::Subscriber sub_aeb = node.subscribe("/aeb", 10, aebCallback); // subscribe to AEB node

   
    ros::spin();

    return 0;

}












