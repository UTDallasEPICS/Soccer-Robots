# Import OpenCV 2 Library (called opencv-python)
import cv2
import numpy as np
# Import library python bindings to apriltag3 library
from pupil_apriltags import Detector

import picamera2
from backtracking import get_direction

# Runs the tracker to backtrack the car to a position
def runTrakcer(destX=200, destY=200):

    # Backtracking algorithm that returns finished code
    isDone = False

    # Initialize the detector
    at_detector = Detector(
    families="tag36h11",
    nthreads=1,
    quad_decimate=1.0,
    quad_sigma=0.0,
    refine_edges=1,
    decode_sharpening=0.25,
    debug=0
    )

    # Create a picamera2 object
    picam2 = picamera2.Picamera2()
    picam2.start()


    while True:
        # Capture frame-by-frame
 
        print("IN DA LOOP CUH")
        frame = picam2.capture_array()

        # Convert the frame to grayscale
        img_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Detect the AprilTags in the frame
        tags = at_detector.detect(img_gray)

        # Display the desired position
        dest_text = f"D POS: ({destX}, {destY})"
        cv2.putText(frame, dest_text, (destX, destY - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2)

        cv2.putText(frame, "+", (destX, destY), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2) 

        # Loop over each detected tag
        for tag in tags:
            # Get the corners of the tag
            corners = tag.corners.astype(int)

            # Draw a green bounding box around the tag
            cv2.polylines(frame, [corners], True, (0, 255, 0), thickness=5)

            # Get the tag center
            cX = int((corners[0, 0] + corners[2, 0]) / 2)
            cY = int((corners[0, 1] + corners[2, 1]) / 2)

            # Put the center coordinates near the tag center
            center_text = "x={:d}, y={:d}".format(cX, cY)
            cv2.putText(frame, center_text, (cX, cY), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            
            # Display the tag id
            id_text = "id={:d}".format(tag.tag_id)
            cv2.putText(frame, id_text, (cX, cY - 40), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            
            # Find out rotation by getting the relation of the center point to the top right point, if the car is facing up, then the rotation is 45 degrees
            # Calculate the rotation of the tag
            
            dx = corners[2, 0] - corners[0, 0]
            dy = corners[2, 1] - corners[0, 1]
            rotation = np.degrees(np.arctan2(dy, dx))
            rotation = (rotation + 45) % 360  # Adjust the rotation by adding 45 degrees

            # Display the rotation
            rotation_text = "rotation={:.2f} degrees".format(rotation)
            cv2.putText(frame, rotation_text, (cX, cY - 80), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            
            movedir = get_direction(cX, cY, rotation, destX, destY)

            if(movedir == -1):
                isDone = True
            '''

            match(movedir):
                case -1:
                    print("We made it!")
                    isDone = True
                case 0b1000:
                    print("Move Forward")
                case 0b0010:
                    print("Move Backward")
                case 0b0100:
                    print("Turn Left")
                case 0b0001:
                    print("Turn Right")

            '''
        # Display the frame with bounding boxes
        cv2.imshow('AprilTags', frame)

        # Break the loop if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q') or isDone:
            break

    # When everything done, release the video capture and video write objects
    picam2.stop()

    # Closes all the frames
    cv2.destroyAllWindows()
    
    return True
