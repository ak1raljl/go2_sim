#ifndef RL_SIM_HPP
#define RL_SIM_HPP

#include "rl_sdk.hpp"
#include "observation_buffer.hpp"
#include "inference_runtime.hpp"
#include "loop.hpp"
#include "fsm_go2.hpp"

#include <csignal>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <ros/ros.h>
#include "std_srvs/Empty.h"
#include <geometry_msgs/Twist.h>
#include <gazebo_msgs/ModelStates.h>
#include "robot_msgs/MotorCommand.h"
#include "robot_msgs/MotorState.h"


class RL_Sim : public RL{
public:
    RL_Sim(int argc, char **argv);
    ~RL_Sim();

private:
    std::vector<float> Forward() override;
    void GetState(RobotState<float> *state) override;
    void SetCommand(const RobotCommand<float> *command) override;
    void RunModel();
    void RobotControl();

    std::shared_ptr<LoopFunc> loop_keyboard;
    std::shared_ptr<LoopFunc> loop_control;
    std::shared_ptr<LoopFunc> loop_rl;
    std::shared_ptr<LoopFunc> loop_plot;

    std::string ros_namespace;

    geometry_msgs::Twist vel;
    geometry_msgs::Pose pose;
    geometry_msgs::Twist cmd_vel;
    ros::Subscriber model_state_subscriber;
    ros::Subscriber cmd_vel_subscriber;
    ros::ServiceClient gazebo_pause_physics_client;
    ros::ServiceClient gazebo_unpause_physics_client;
    ros::ServiceClient gazebo_reset_world_client;
    std::map<std::string, ros::Publisher> joint_publishers;
    std::map<std::string, ros::Subscriber> joint_subscribers;
    std::vector<robot_msgs::MotorCommand> joint_publishers_commands;
    void ModelStatesCallback(const gazebo_msgs::ModelStates::ConstPtr &msg);
    void JointStatesCallback(const robot_msgs::MotorState::ConstPtr &msg, const std::string &joint_controller_name);
    void CmdvelCallback(const geometry_msgs::Twist::ConstPtr &msg);

    std::string gazebo_model_name;
    std::map<std::string, float> joint_positions;
    std::map<std::string, float> joint_velocities;
    std::map<std::string, float> joint_efforts;
    void StartJointController(const std::string& ros_namespace, const std::vector<std::string>& names);
};

#endif // RL_SIM_HPP
