#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <ackermann_msgs/AckermannDriveStamped.h>

float speed_limit = 4.0;
float angle_limit = 0.3;
float speed = 1.0;
float angle = 0.0;
ros::Publisher command_pub;

void scan_callback(const sensor_msgs::LaserScan::ConstPtr& laser_scan) {
    int start_index = 330, end_index = 740;
    float distance_threshold = 3;
    int start_threshold_index = start_index, end_threshold_index = end_index;
    float angle_min = laser_scan->angle_min, angle_increment = laser_scan->angle_increment;

    for (int i = start_index; i <= end_index; ++i) {
        if (laser_scan->ranges[i] >= distance_threshold) {
            start_threshold_index = i;
            break;
        }
    }
    for (int i = start_threshold_index; i < end_index; ++i) { 
        if (laser_scan->ranges[i] <= distance_threshold) {
            end_threshold_index = i;
            break;
        }
    }

    angle = (start_threshold_index + end_threshold_index) * 0.5f * angle_increment + angle_min;
}

int main(int argc, char *argv[]) {
    ros::init(argc, argv, "gapmerv");
    ros::NodeHandle n;
    ros::Subscriber scan_sub = n.subscribe("/scan", 10, scan_callback);
    ros::Rate loop_rate(50);
    command_pub = n.advertise<ackermann_msgs::AckermannDriveStamped>("/gap_follow", 1);

    while (ros::ok()) {
        ackermann_msgs::AckermannDriveStamped drive_st_msg;
        drive_st_msg.header.stamp = ros::Time::now();
        drive_st_msg.drive.speed = speed * speed_limit;
        drive_st_msg.drive.steering_angle = angle * angle_limit;

        command_pub.publish(drive_st_msg);
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}

