# Unscented Kalman Filter Project
Self-Driving Car Engineer Nanodegree Program


## Outline
This progam implements sensor fusion through an unscented Kalman Filter implmentation that fuses Laser data with radar data to track the 2D cartesian position of a target using the CTRV model.

The prediction step of the filter was processed through a CTRV motion model with five states, x-,y- coordinates, tangential speed, yaw, and yaw rate.

Radar measurements and Lidar measurments were processed through individual functions due to the unscented transform.

The acceleration standard deviation was tuned to 3m/s^2, and the yaw acceleration was tuned to 0.5rad/s^2.
## Dependencies

This code depends on
```bash
libuv1-dev libssl-dev gcc g++ cmake make
```
and uWebSockets
```
https://github.com/uWebSockets/uWebSockets 
```

The simulator used for this program is available [here](https://github.com/udacity/self-driving-car-sim/releases)

## Results

The code successfully compiled and produced RMSE errors within the thresholds of [.09, .10, .40, .30].
