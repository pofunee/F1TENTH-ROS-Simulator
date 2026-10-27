#include "ros/ros.h"
#include "std_msgs/String.h"
#include <ackermann_msgs/AckermannDriveStamped.h>
#include <sensor_msgs/LaserScan.h>
#include <nav_msgs/Odometry.h>
#include <vector>
#include <cmath>

// ROS Publisher for emergency braking
ros::Publisher emergency_brake_pub;

// Global constants
const float EMERGENCY_STOP_SPEED = 0.0;
const float REVERSE_DRIVE_SPEED = -1.2;
const float SAFE_TTC_THRESHOLD = 0.3;
const float PI = 3.14;
const int LIDAR_START_INDEX = 450;
const int LIDAR_END_INDEX = 630;
const float LIDAR_CENTER = 540.0;

// Global variables
double vehicle_speed = 0.0;
std::vector<float> lidar_angles(LIDAR_END_INDEX - LIDAR_START_INDEX + 1, 0.0);
std::vector<float> obstacle_distances(LIDAR_END_INDEX - LIDAR_START_INDEX + 1, 0.0);
std::string driving_mode = "normal";

// Function to compute TTC and apply emergency braking if necessary
void compute_TTC(const nav_msgs::Odometry::ConstPtr& odom_msg) {
    vehicle_speed = odom_msg->twist.twist.linear.x; // Extract current speed

    if (vehicle_speed == 0.0) return;  // Avoid division by zero and unnecessary calculations

    ackermann_msgs::AckermannDriveStamped drive_msg;
    drive_msg.header.stamp = ros::Time::now();

    bool obstacle_detected = false;

 // Loop through LiDAR scan data to check for potential collisions
    for (size_t i = 0; i < obstacle_distances.size(); ++i) {
        float projected_velocity = std::cos(lidar_angles[i]) * vehicle_speed;
        if (projected_velocity <= 0) continue;  // Ignore obstacles behind the vehicle

        float ttc = obstacle_distances[i] / projected_velocity;// Compute TTC
        if (ttc <= SAFE_TTC_THRESHOLD) {// Check if TTC is below safe threshold
            ROS_WARN("Obstacle detected! Index: %zu, Distance: %.2f, Speed: %.2f, TTC: %.2f",
                     i, obstacle_distances[i], projected_velocity, ttc);
          // Stop the vehicle immediately
            drive_msg.drive.speed = EMERGENCY_STOP_SPEED;
            emergency_brake_pub.publish(drive_msg);
            driving_mode = "stop";
            obstacle_detected = true;
            break;
        }
    }

// If no obstacles detected but the vehicle was previously stopped, attempt to reverse
    if (!obstacle_detected && driving_mode == "stop") {
        // Attempt to reverse after stopping
        ROS_WARN("Initiating reverse maneuver...");
        drive_msg.drive.speed = REVERSE_DRIVE_SPEED;
        emergency_brake_pub.publish(drive_msg);
        driving_mode = "normal"; // Reset driving mode to normal
    }
}

// Callback function to process LiDAR data
void lidar_callback(const sensor_msgs::LaserScan::ConstPtr& scan_msg) {
    for (size_t i = 0; i < obstacle_distances.size(); ++i) {
        int lidar_index = i + LIDAR_START_INDEX;
        lidar_angles[i] = PI * (lidar_index - LIDAR_CENTER) / LIDAR_CENTER;// Convert index to angle
        obstacle_distances[i] = scan_msg->ranges[lidar_index];// Extract distance values
    }
}

// Main function
int main(int argc, char **argv) {
    ros::init(argc, argv, "aeb_merv");
    ros::NodeHandle nh;
// Define emergency braking publisher
    emergency_brake_pub = nh.advertise<ackermann_msgs::AckermannDriveStamped>("/aeb", 10);
 // Subscribe to LiDAR and odometry topics
    ros::Subscriber lidar_sub = nh.subscribe("/scan", 10, lidar_callback);
    ros::Subscriber odom_sub = nh.subscribe("/odom", 10, compute_TTC);

    ros::spin();
    return 0;
}
