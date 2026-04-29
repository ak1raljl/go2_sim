# go2_control

## Compile

```bash
cd /go2_control
catkin build
```

## Run

Terminal 1:

```bash
source devel/setup.bash
roslaunch go2_sim gazebo.launch rname:=go2
```

Terminal 2:

```bash
source devel/setup.bash
rosrun go2_sim rl_sim _rl_config_name:=go2_cts
```

## Keyboard

FSM keys:

| Key | Current state | Action |
| --- | --- | --- |
| `0` | `Passive` | Switch to `GetUp` |
| `1` | `GetUp` completed | Switch to `RLLocomotion` |
| `9` | `GetUp` completed / `RLLocomotion` | Switch to `GetDown` |
| `P` | `GetUp` / `GetDown` / `RLLocomotion` | Switch to `Passive` |
| `0` | `GetDown` | Interrupt get-down and switch to `GetUp` |

Common flow:

```text
start rl_sim -> press 0 -> wait for get-up -> press 1 -> RL control
```

Velocity command keys:

| Key | Command |
| --- | --- |
| `W` | `control.x = cmd_range[0][1]` |
| `S` | `control.x = cmd_range[0][0]` |
| `A` | `control.y = cmd_range[1][1]` |
| `D` | `control.y = cmd_range[1][0]` |
| `Q` | `control.yaw = cmd_range[2][1]` |
| `E` | `control.yaw = cmd_range[2][0]` |
| `Space` | `control.x/y/yaw = 0` |

`cmd_range` is configured in `policy/go2/base.yaml`:

```yaml
cmd_range:
  - [-1.0, 1.0]   # vx: S, W
  - [-0.8, 0.8]   # vy: D, A
  - [-1.0, 1.0]   # yaw: E, Q
```

The policy receives:

```cpp
obs.commands = {control.x, control.y, control.yaw};
```

Extra keys:

| Key | Action |
| --- | --- |
| `R` | Reset Gazebo world |
| `Enter` | Pause / unpause Gazebo physics |
| `N` | Toggle navigation mode |

When navigation mode is on, commands come from `/cmd_vel` instead of keyboard:

```cpp
obs.commands = {
  cmd_vel.linear.x,
  cmd_vel.linear.y,
  cmd_vel.angular.z
};
```
