package client;

import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Scanner;

public class Menu {
    private static final Charset ENC = StandardCharsets.US_ASCII;
    private static final byte LOC_GET = 'A';
    private static final byte LOC_UPDATE = 'B';
    private static final byte LOC_FRIEND = 'C';
    private static final byte FRIEND_GET = 'D';
    private static final byte FRIEND_SEND = 'E';
    private static final byte FRIEND_ACCEPT = 'F';
    private static final byte FRIEND_REMOVE = 'G';

    public static void displayMenu(User u){
        System.out.println("\nEnter an option, "+ u.getEmail());
        System.out.println("A) Get my current location");
        System.out.println("B) Update my location");
        System.out.println("C) View a Friend's location");
        System.out.println("D) View pending Friend Requests");
        System.out.println("E) Send a Friend Request");
        System.out.println("F) Accept a Friend Request");
        System.out.println("G) Remove a friend");
        System.out.println("H) Close App");
    }

    public static byte[] handleOption(char c, User user, Scanner reader) throws IOException, IllegalArgumentException {
        if(c == LOC_GET){
            return DataProtocolEncoder.getLocation(user);
        } else if( c == LOC_UPDATE){
            System.out.println("Enter your lattitude");
            Float lat = reader.nextFloat();
            System.out.println(lat);
            System.out.println("Enter your longitude");
            Float longitude = reader.nextFloat();
            System.out.println(longitude);
            /*TODO Error Check*/
            user.setLocation(new Location(lat, longitude));
            return DataProtocolEncoder.updateLocation(user);
        } else if( c == LOC_FRIEND){
            System.out.println("Enter the email of your friend");
            String friendName = reader.next();
            return DataProtocolEncoder.getFriendLocation(user, friendName);
        } else if( c == FRIEND_GET){
            return DataProtocolEncoder.getFriendRequest(user);
        } else if( c == FRIEND_SEND){
            System.out.println("Enter the email of your friend to send");
            String friendName= reader.next();
            return DataProtocolEncoder.sendFriendRequest(user, friendName);
        } else if( c == FRIEND_ACCEPT){
            System.out.println("Enter the email of your friend to accept");
            String friendName= reader.next();
            return DataProtocolEncoder.acceptFriendRequest(user, friendName);
        } else if( c == FRIEND_REMOVE){
            System.out.println("Enter the email of your friend");
            String friendName= reader.next();
            return DataProtocolEncoder.removeFriend(user, friendName);
        }
        else{
            throw new IllegalArgumentException();
        }
    }

    public static void handleResponse(char c, User user, byte[] serverResponse){
        byte myResponse = DataProtocolDecoder.getResponse(serverResponse);
        if(c == LOC_GET){
            Location loc = DataProtocolDecoder.retrieveLocation(serverResponse);
            if(loc != null){
                System.out.println("Your location is " + loc.getLattitude() +", " +loc.getLongitude());
                user.setLocation(loc);
            } else {
                System.out.println("\nLocation could not be found at this time");
            }
        } else if( c == LOC_UPDATE){
            if(myResponse==1)
                System.out.println("\nLocation Updated");
            else
                System.out.println("\nError, invalid location");
        } else if( c == LOC_FRIEND){
                if(myResponse == 1){
                    Location location = DataProtocolDecoder.retrieveLocation(serverResponse);
                    System.out.println("\nYour friend is at "+ location.getLattitude()+", " + location.getLongitude());
                }
		else{
		    System.out.println("\nFriend not found, or you are not friends with this person");
		}
        } else if( c == FRIEND_GET){
            if(myResponse==1){
		    System.out.println("\nYou have friend requests from: ");
                ArrayList<String> myFriends = DataProtocolDecoder.getFriends(serverResponse);
                for(String f : myFriends){
                    System.out.println(f);
                }
            }
	    else{
	       System.out.println("\nNo active friend requests!");
	    }
        } else if( c == FRIEND_SEND){
            if(myResponse == 1){
                System.out.println("\nFriend Request Sent!");
            }else if(myResponse == 2){
                System.out.println("\nThe name you entered is already your friend!");
            }else{
	        System.out.println("\nUser not found!");
	    }
        } else if( c == FRIEND_ACCEPT){
            if(myResponse == 1){
                System.out.println("\nAccepted!");
            }else if(myResponse == 2){
                System.out.println("\nThe name you entered is already your friend!");
            }else{
	        System.out.println("\nUser not found, or has not sent you a friend request!");
	    }
        } else if( c == FRIEND_REMOVE){
            if(myResponse == 1){
                System.out.println("\nFriend Removed!");
            }else{
                System.out.println("\nError");
            }
        } else{
            throw new UnsupportedOperationException();
        }
    }
    public static int getExpectedReturnSize(char c){
        if(c == LOC_GET){
            return 9;
        } else if( c == LOC_UPDATE){
            return 1;
        } else if( c == LOC_FRIEND){
            return 9;
        } else if( c == FRIEND_GET){
            return 100;
        } else if( c == FRIEND_SEND){
            return 1;
        } else if( c == FRIEND_ACCEPT){
            return 1;
        } else if( c == FRIEND_REMOVE){
            return 1;
        }
        else{
            throw new IllegalArgumentException();
        }
    }
}
