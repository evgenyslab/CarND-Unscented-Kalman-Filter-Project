#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 * This is scaffolding, do not modify
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;


  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 9;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 6;
  
  //DO NOT MODIFY measurement noise values below these are provided by the sensor manufacturer.
  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;
  //DO NOT MODIFY measurement noise values above these are provided by the sensor manufacturer.
  
  /**
  TODO:

  Complete the initialization. See ukf.h for other member properties.

  Hint: one or more values initialized above might be wildly off...
  */
    // Initialization:
    is_initialized_ = false;
    // States:
    n_x_ = 5;
    // Augmented states:
    n_aug_ = 7;
    //define spreading parameter
    lambda_ = 3 - n_aug_;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Make sure you switch between lidar and radar
  measurements.

   Initialize
   Predict
   Update
  */
    // 1.0 INITIALIZE
    if (!is_initialized_){
        // first measurement
        if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
            /**
            Convert radar from polar to cartesian coordinates and initialize state.
            */
            x_ <<  meas_package.raw_measurements_[0]*cos(meas_package.raw_measurements_[1]),
                    meas_package.raw_measurements_[0]*sin(meas_package.raw_measurements_[1]),
                    0,0,0;
        }
        else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
            /**
            Initialize state.
            */
            x_ << meas_package.raw_measurements_[0], meas_package.raw_measurements_[1], 0,0,-;

        }
        // set previous time stamp:
        previous_timestamp_ = meas_package.timestamp_;
        // initialize covariance matrix:

        P_ <<   1, 0, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 1, 0,
                0, 0, 0, 0, 1;

        is_initialized_ = true;
        return;

    }
    // 2.0 PREDICT
    float delta_t_ = (meas_package.timestamp_ - previous_timestamp_) / 1000000.0;	//dt - expressed in seconds
    previous_timestamp_ = meas_package.timestamp_;

    /* 2.1 Generate Sigma Points:
     *
     * Generate sigma points using augmentation.
     * */
    //create augmented mean vector
    VectorXd x_aug = VectorXd(7);

    //create augmented state covariance
    MatrixXd P_aug = MatrixXd(7, 7);

    //create sigma point matrix
    MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);

    //create augmented mean state
    x_aug.head(5) = x_;
    x_aug(5) = 0;
    x_aug(6) = 0;

    //create augmented covariance matrix
    P_aug.fill(0.0);
    P_aug.topLeftCorner(5,5) = P_;
    P_aug(5,5) = std_a_*std_a_;
    P_aug(6,6) = std_yawdd_*std_yawdd_;

    //create square root matrix
    MatrixXd L = P_aug.llt().matrixL();

    //create augmented sigma points
    Xsig_aug.col(0)  = x_aug;
    for (int i = 0; i< n_aug_; i++)
    {
        Xsig_aug.col(i+1)       = x_aug + sqrt(lambda_+n_aug_) * L.col(i);
        Xsig_aug.col(i+1+n_aug_) = x_aug - sqrt(lambda_+n_aug_) * L.col(i);
    }

    /* 2.2 Predict Sigma Points:
     * TODO: MOVE TO SEPARATE FUNCTION BELOW!
     * */
    //create matrix with predicted sigma points as columns
    Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);

    //predict sigma points
    for (int i = 0; i< 2*n_aug_+1; i++)
    {
        //extract values for better readability
        double p_x = Xsig_aug(0,i);
        double p_y = Xsig_aug(1,i);
        double v = Xsig_aug(2,i);
        double yaw = Xsig_aug(3,i);
        double yawd = Xsig_aug(4,i);
        double nu_a = Xsig_aug(5,i);
        double nu_yawdd = Xsig_aug(6,i);

        //predicted state values
        double px_p, py_p;

        //avoid division by zero
        if (fabs(yawd) > 0.001) {
            px_p = p_x + v/yawd * ( sin (yaw + yawd*delta_t_) - sin(yaw));
            py_p = p_y + v/yawd * ( cos(yaw) - cos(yaw+yawd*delta_t_) );
        }
        else {
            px_p = p_x + v*delta_t_*cos(yaw);
            py_p = p_y + v*delta_t_*sin(yaw);
        }

        double v_p = v;
        double yaw_p = yaw + yawd*delta_t_;
        double yawd_p = yawd;

        //add noise
        px_p = px_p + 0.5*nu_a*delta_t_*delta_t_ * cos(yaw);
        py_p = py_p + 0.5*nu_a*delta_t_*delta_t_ * sin(yaw);
        v_p = v_p + nu_a*delta_t_;

        yaw_p = yaw_p + 0.5*nu_yawdd*delta_t_*delta_t_;
        yawd_p = yawd_p + nu_yawdd*delta_t_;

        //write predicted sigma point into right column
        Xsig_pred_(0,i) = px_p;
        Xsig_pred_(1,i) = py_p;
        Xsig_pred_(2,i) = v_p;
        Xsig_pred_(3,i) = yaw_p;
        Xsig_pred_(4,i) = yawd_p;
    }


    /* 2.3 Predict Means and Covariance
     *
     * */
    //create vector for weights
    VectorXd weights = VectorXd(2*n_aug_+1);

    //create vector for predicted state
    VectorXd x = VectorXd(n_x_);

    //create covariance matrix for prediction
    MatrixXd P = MatrixXd(n_x_, n_x_);

    // set weights
    double weight_0 = lambda_/(lambda_+n_aug_);
    weights(0) = weight_0;
    for (int i=1; i<2*n_aug_+1; i++) {  //2n+1 weights
        double weight = 0.5/(n_aug_+lambda_);
        weights(i) = weight;
    }

    //predicted state mean
    x.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points
        x = x+ weights(i) * Xsig_pred_.col(i);
    }

    //predicted state covariance matrix
    // TODO: CHECK THIS -> Shoud P be empty??
    P.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points

        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x;
        //angle normalization
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

        P = P + weights(i) * x_diff * x_diff.transpose() ;
    }


    /* 3.0 Update
     *
     * */


    /* 3.1 Predict Measurement
     *
     * */
    if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
        //
        //create matrix for sigma points in measurement space
        // TODO: SET n_z)
        int n_z_ = 3;
        MatrixXd Zsig = MatrixXd(n_z_, 2 * n_aug_ + 1);
        //transform sigma points into measurement space
        for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points

            // extract values for better readibility
            double p_x = Xsig_pred_(0,i);
            double p_y = Xsig_pred_(1,i);
            double v  = Xsig_pred_(2,i);
            double yaw = Xsig_pred_(3,i);

            double v1 = cos(yaw)*v;
            double v2 = sin(yaw)*v;

            // measurement model
            Zsig(0,i) = sqrt(p_x*p_x + p_y*p_y);                        //r
            Zsig(1,i) = atan2(p_y,p_x);                                 //phi
            Zsig(2,i) = (p_x*v1 + p_y*v2 ) / sqrt(p_x*p_x + p_y*p_y);   //r_dot
        }

        //mean predicted measurement
        VectorXd z_pred = VectorXd(n_z_);
        z_pred.fill(0.0);
        for (int i=0; i < 2*n_aug_+1; i++) {
            z_pred = z_pred + weights(i) * Zsig.col(i);
        }

        //innovation covariance matrix S
        MatrixXd S = MatrixXd(n_z_,n_z_);
        S.fill(0.0);
        for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
            //residual
            VectorXd z_diff = Zsig.col(i) - z_pred;

            //angle normalization
            while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
            while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

            S = S + weights(i) * z_diff * z_diff.transpose();
        }

        //add measurement noise covariance matrix
        MatrixXd R = MatrixXd(n_z_,n_z_);
        R <<    std_radr_*std_radr_, 0, 0,
                0, std_radphi_*std_radphi_, 0,
                0, 0,std_radrd_*std_radrd_;
        S = S + R;
        /* 3.1. Update RADAR:
         *
         * */
        //create matrix for cross correlation Tc
        MatrixXd Tc = MatrixXd(n_x_, n_z_);
        //calculate cross correlation matrix
        Tc.fill(0.0);
        for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points

            //residual
            VectorXd z_diff = Zsig.col(i) - z_pred;
            //angle normalization
            while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
            while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

            // state difference
            VectorXd x_diff = Xsig_pred_.col(i) - x;
            //angle normalization
            while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
            while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

            Tc = Tc + weights(i) * x_diff * z_diff.transpose();
        }

        //Kalman gain K;
        MatrixXd K = Tc * S.inverse();

        // Measurement
        VectorXd z(n_z_);
        z << meas_package.raw_measurements_[0] , meas_package.raw_measurements_[1],meas_package.raw_measurements_[2];
                //residual
        VectorXd z_diff = z - z_pred;

        //angle normalization
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;

        //update state mean and covariance matrix
        x = x + K * z_diff;
        // TODO: CHECK P vs P_!!
        P_ = P_ - K*S*K.transpose();


    }
    else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
       // TODO: copy from above, bot only 2 states! px, py!
    }

    /* 3.2 Update State
     *
     * */


}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
  /**
  TODO:

  Complete this function! Estimate the object's location. Modify the state
  vector, x_. Predict sigma points, the state, and the state covariance matrix.
  */
}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use lidar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the lidar NIS.
  */
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use radar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the radar NIS.
  */
}
