package client;

public class Location {

    private float lattitude;
    private float longitude;

    public Location(){
        this.lattitude = 0;
        this.longitude = 0;
    }

    public Location(float lat, float longitude){
        this.lattitude = lat;
        this.longitude= longitude;
    }
    public float getLattitude() {
        return lattitude;
    }

    public float getLongitude() {
        return longitude;
    }
}
