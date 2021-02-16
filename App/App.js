//React Native Button
//https://aboutreact.com/react-native-button/

//import React in our code
import React, {useEffect, useState, useRef} from 'react';

//import all the components we are going to use
import { Image, StyleSheet, View} from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';
import {Button, Input} from "react-native-elements";

import { loading } from './components/loading'
import logo from "./assets/mydogs_logo.png";
import Icon from "react-native-vector-icons/FontAwesome5";
import { Base64 } from "./components/base64"

// Notification
import Constants from 'expo-constants';
import * as Notifications from 'expo-notifications';


Notifications.setNotificationHandler({
  handleNotification: async () => ({
    shouldShowAlert: true,
    shouldPlaySound: false,
    shouldSetBadge: false,
  }),
});

const App = () => {

  let [webcam, setWebcam] = useState(loading);
  let ws;

  const [ip, setIp] = useState();



  const [expoPushToken, setExpoPushToken] = useState('');
  const [notification, setNotification] = useState(false);
  const notificationListener = useRef();
  const responseListener = useRef();

  const webSocket = () => {

    ws = new WebSocket('ws://'+ip+':1337');

    ws.onopen = () => {
      alert("Connected")
    }
    ws.onclose = e => {
      // connection closed
      alert("Error Code : "+e.code);
    };
    ws.onmessage = async e => {
      // a message was received
      setImageWebcam(e)



    };
  }

  const setImageWebcam = async e => {
    let bytes = new Uint8Array(e.data);
    let binary = '';
    let len = bytes.byteLength;
    for (let i = 0; i < len; i++) {
      binary += String.fromCharCode(bytes[i])
    }
    let tempCam = 'data:image/jpg;base64,'+Base64.btoa(binary);
    setWebcam(tempCam)
  }

  const giveCroquette = () => {
    ws.send('-')
  }

  const save = async () => {
    try {
      await AsyncStorage.setItem("ipWebSocket", ip);
      webSocket();
    }catch (e) {
      alert(e);
    }

  };

  const load = async () => {
    try {
      let ip = await AsyncStorage.getItem("ipWebSocket");

      if (ip !== null) {
        setIp(ip);
      }else{
        setIp('192.168.0.1')
      }

    }catch (e) {
      alert(e)
    }
  }

  useEffect(() => {

    registerForPushNotificationsAsync().then(token => setExpoPushToken(token));

    notificationListener.current = Notifications.addNotificationReceivedListener(notification => {
      setNotification(notification);
    });

    responseListener.current = Notifications.addNotificationResponseReceivedListener(response => {
      console.log(response);
    });

    load()

    return () => {
      Notifications.removeNotificationSubscription(notificationListener);
      Notifications.removeNotificationSubscription(responseListener);
    };

  }, [])



  return (
        <View style={styles.container}>
          <View style={styles.logoContainer}>
            <Image source={logo} style={{maxWidth: 200, maxHeight: 200 }} />
          </View>
          <View style={styles.formIp}>
            <Input
              placeholder={ip}
              leftIcon={
                <Icon
                  name='globe'
                  size={16}
                  color='black'
                  />
              }
              style={{height: 50}}
              onChangeText={text => setIp(text)}
            />
            <Button onPress={save} title="Go" style={styles.button} />
          </View>
          <View style={styles.main}>
            <Image source={{uri: webcam}} style={{width: 460, height: 240}}/>
            <Button onPress={giveCroquette} title="Donner une croquette" style={styles.button} />
          </View>
        </View>
  );
};
const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'space-around',
  },
  logoContainer: {
    marginTop: 30,
    flex: 2,
    alignItems: 'center'
  },
  main:{
    flex: 6,
    justifyContent: 'space-evenly',
    alignItems: 'center'
  },
  formIp: {
    flex: 1,
    flexDirection: 'row',
    justifyContent: 'center',
    width: '100%',
    padding: 40,
    height: 50
  },
  button: {
    paddingHorizontal: 10,
    paddingVertical: 4

  }
});
export default App;

async function registerForPushNotificationsAsync() {
  let token;
  if (Constants.isDevice) {
    const { status: existingStatus } = await Notifications.getPermissionsAsync();
    let finalStatus = existingStatus;
    if (existingStatus !== 'granted') {
      const { status } = await Notifications.requestPermissionsAsync();
      finalStatus = status;
    }
    if (finalStatus !== 'granted') {
      alert('Failed to get push token for push notification!');
      return;
    }
    token = (await Notifications.getExpoPushTokenAsync()).data;
    console.log(token);
  } else {
    alert('Must use physical device for Push Notifications');
  }

  if (Platform.OS === 'android') {
    await Notifications.setNotificationChannelAsync('default', {
      name: 'default',
      importance: Notifications.AndroidImportance.MAX,
      vibrationPattern: [0, 250, 250, 250],
      lightColor: '#FF231F7C',
    });
  }
  console.log(token);
  return token;
}
