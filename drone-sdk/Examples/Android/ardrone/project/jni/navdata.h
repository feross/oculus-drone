#ifndef _NAVDATA_H_
#define _NAVDATA_H_

#include <ardrone_api.h>
#include <VP_Os/vp_os_types.h>

typedef struct _instance_navdata_t 
{
		uint32_t ardrone_state;
		bool_t flyingState;		/// Tells whether ARDrone is flying or not
		bool_t commandState;		/// ???
		bool_t comWatchdog;		/// Tells if we are loosing communication with ARDrone
		/// Be aware that at startup, comWatchdog is set to 1 because no input reach the drone yet
		bool_t emergencyLanding;	/// Tells ARDrone landed because of a serious reason
		bool_t timerElapsed;	    /// Tells ARDrone can take off again
		bool_t startPressed;		/// Start button state by ARDrone
		bool_t bootstrap;			/// mykonos is starting
		
		unsigned int remainingBattery; /// Remaining battery in mV
		
		float pitch;			/// cabrement en radians (nez vers le bas negatif)
		float roll;				/// roulis en radians (pencher a gauche = negatif)
		float yaw;				/// lacet en radians (tourner vers la gauche = negatif)
		
		float trimPitch;			/// trim on pitch
		float trimRoll;				/// trim on roll
		float trimYaw;				/// trim on yaw
		
		float vx;				// Horizontal velocity on x-axis
		float vy;				// Horizontal velocity on y-axis
		float vz;				// Horizontal velocity on y-axis
		
		float altitude;			/// altitude en m
		
		int nbDetectedOpponent;	/// Does we see an opponent?
		int	xOpponent[4];			/// Position (in screen space) of the opponent
		int yOpponent[4];
		int	widthOpponent[4];			/// Position (in screen space) of the opponent
		int heightOpponent[4];	
		int distOpponent[4];		/// Distance from the opponent
		
		// Video num frames
		int videoNumFrames;
		
		// Camera parameters compute by detection
		float detectCameraRot[3][3];
		float detectCameraHomo[3][3];
		float detectCameraTrans[3];
		int	  detectTagIndex;
		
		// Type of detection
		int   detectionType;

		// Camera parameters compute by drone
		float droneCameraRot[3][3];
		float droneCameraTrans[3];
		
		bool_t cutoutProblem;
		bool_t cameraProblem;
		bool_t adcVersionProblem;
		bool_t adcProblem;
		bool_t ultrasoundProblem;
		bool_t visionProblem;
		bool_t anglesProblem;
		bool_t vbatLowProblem;
		bool_t motorsProblem;
		bool_t userEmergency;
		
		bool_t videoThreadOn;
		bool_t navdataThreadOn;
} instance_navdata_t;
C_RESULT ardrone_navdata_reset_data(instance_navdata_t *nav);
C_RESULT ardrone_navdata_get_data(instance_navdata_t *data);
C_RESULT ardrone_navdata_write_to_file(bool_t enable);

#endif // _NAVDATA_H_
