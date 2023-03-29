echo autospawn = no > $HOME/.config/pulse/client.conf
pulseaudio --kill
read -p "Press enter to enable pulseaudio again or any other one to left it stopped."
rm $HOME/.config/pulse/client.conf
pulseaudio --start
