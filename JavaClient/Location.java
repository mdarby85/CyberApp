/**
 * file: Location.java
 * author: Group 2
 * Date Modified: 11/23/19
 * This program represents a user's location
 */
package client;

public class Location {

    private float lattitude;
    private float longitude;

    /**
     * Default Location constructor
     */
    public Location(){
        this.lattitude = 0;
        this.longitude = 0;
    }

    /**
     * Location constructor with intial values
     * @param lat Initial lattitude
     * @param longitude Initial Longitude
     */
    public Location(float lat, float longitude){
        this.lattitude = lat;
        this.longitude= longitude;
    }

    /**
     * Default getter for lattitude
     * @return lattitude
     */
    public float getLattitude() {
        return lattitude;
    }

    /**
     * Default getter for longitude
     * @return longitude
     */
    public float getLongitude() {
        return longitude;
    }
}
