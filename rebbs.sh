tail -n 1 ~bbs/run/mbbsd.pid | awk '{print $1}' | xargs kill -9
echo "The process has been killed."

echo "Now mbbsd is running in "$1.
~bbs/bin/camera
~bbs/bin/mbbsd $1
