var arDrone = require('ar-drone')
var fs = require('fs')
var net = require('net')
var optimist = require('optimist')
var PaVEParser = require('./node_modules/ar-drone/lib/video/PaVEParser')
var PidController = require('node-pid-controller')
var split = require('split')
var through = require('through')

// Don't crash node on exceptions
process.on('uncaughtException', function (err) {
  console.error(err.stack)
  try {
    land()
  } catch (e) {}
  setTimeout(function () {
    process.exit(0)
  }, 200)
})

var argv = optimist
  .usage('Usage: $0 [--oculus]')
  .alias('oculus', 'o')
  .argv

var drone = arDrone.createClient()
var speed = {
  x: 0, // forward/back
  y: 0, // left/right
  z: 0, // rotateLeft/rotateRight
  e: 0  // up/down
}
var pids = {
  x: new PidController(0.05, 0.00004, 0.0004),
  y: new PidController(0.05, 0.00004, 0.0004),
  z: new PidController(0.05, 0.00004, 0.0004)
}
var zero = {
  x: 0,
  y: 0,
  z: 0
}
var lastOutput = {
  x: 0,
  y: 0,
  z: 0
}
var lastInput = {
  x: 0,
  y: 0,
  z: 0
}

function correct(zTarget, last) {
      while(zTarget - last.z > 180) {
        zTarget -= 360
      }
      while(last.z - zTarget > 180) {
        zTarget += 360
      }
      last.z = zTarget
      return zTarget
}

var inAir = false, isRunning = false
var gestureEnabled = true

drone.disableEmergency()
drone.stop() // Stop command drone was executing before batt died
drone.config('control:control_yaw', '6.1')
drone.config('control:euler_angle_max', '0.5')

drone.on('batteryChange', function (num) {
  console.log('battery: ' + num)
})

setInterval(function () {
  console.log(speed)
  console.log({
    xTarget: pids.x.target.toFixed(2),
    yTarget: pids.y.target.toFixed(2),
    zTarget: pids.z.target.toFixed(2)
  })
}, 1000)

function set (params) {
  for (var param in params) {
    var val = params[param]

    // Update parameter
    if (val > 1) {
      speed[param] = 1
    } else if (val < -1) {
      speed[param] = -1
    } else {
      speed[param] = val
    }

    // Send movement command to drone
    if (param === 'x') {
      drone.front(val)
    } else if (param === 'y') {
      drone.right(val)
    } else if (param === 'e') {
      drone.up(val)
    } else if (param === 'z') {
      drone.clockwise(val)
    } else {
      console.error('Invalid param to `set`')
    }
  }
}

function reset () {
  set({x: 0, y: 0, z: 0, e: 0})
  drone.stop()
}

function takeoff () {
  drone.disableEmergency()
  drone.stop()
  drone.takeoff()
  inAir = true
  setTimeout(function(){isRunning = true}, 1000)
}

function land () {
  inAir = false
  drone.land()
  setTimeout(function(){isRunning = false}, 1000)
}

function flipAhead () {
  drone.animate('flipAhead', 500)
}
function flipBehind () {
  drone.animate('flipBehind', 500)
}
function flipLeft () {
  drone.animate('flipLeft', 500)
}
function flipRight () {
  drone.animate('flipRight', 500)
}

function disableGestureTimeout () {
  gestureEnabled = false
  setTimeout(function () {
    gestureEnabled = true
  }, 2000)
}

// Keyboard control
process.stdin.setRawMode(true)
process.stdin.on('data', function(chunk) {
  var key = chunk.toString()
  var keyBuf = chunk.toJSON()
  var speed = 0.2

  console.log(key)
  console.log(keyBuf)

  if (Array.isArray(keyBuf)) {
    var UP = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 65)
    var DOWN = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 66)
    var RIGHT = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 67)
    var LEFT = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 68)
  }

  if (key === 'w') {
    set({ x: speed })
  } else if (key === 's') {
    set({ x: -speed })
  } else if (key === 'd') {
    set({ y: speed })
  } else if (key === 'a') {
    set({ y: -speed })
  } else if (UP) {
    set({ e: speed })
  } else if (DOWN) {
    set({ e: -speed })
  } else if (LEFT) {
    set({ z: -speed }) // COUNTERCLOCKWISE
  } else if (RIGHT) {
    set({ z: speed }) // CLOCKWISE
  } else if (key === 'e') {
    drone.stop()
  } else if (key === 'k') {
    land()
    setTimeout(function () {
      process.exit(0)
    }, 200)
  } else if (key === 't') {
    takeoff()
  } else if (key === 'l') {
    land()

  } else if (keyBuf[0] === 32) {
    flipAhead()
  }
})

// Oculus control
if (argv.oculus) {

  // Connect to oculus server
  var oculusClient = net.connect({ port: 12345 })
  oculusClient.on('error', function (err) {
    console.error('Error connecting to oculus server.')
  })

  // Process oculus data
  oculusClient
    .pipe(split())
    .pipe(through(function (line) {
      var arr = line.split(',')
      var xTarget = Number(-arr[0])
      var yTarget = Number(-arr[2])
      var zTarget = Number(-arr[1] * (180 / Math.PI))
      pids.z.setTarget(correct(zTarget, lastOutput))

      // Gestures:
      //   DOWN to takeoff/land
      //   UP to flip
      // if (gestureEnabled) {
      //   if (xTarget < -1) {
      //     flipBehind()
      //     disableGestureTimeout()
      //   } else if (xTarget > 1) {
      //     if (inAir) {
      //       land()
      //     } else {
      //       takeoff()
      //     }
      //     disableGestureTimeout()
      //   }
      // }
    }))
}

// Video server
net.createServer(function (c) {
  console.log('server connected')

  c.on('end', function() {
    console.log('server disconnected')
  })

  drone.getVideoStream()
    .pipe(new PaVEParser())
    .pipe(through(function (data) {
      this.queue(data.payload)
      console.log('video bits sent')
    }))
    .pipe(c)

}).listen(6969)

drone.on('navdata', function (data) {
  var xSensor = Number(data.demo.rotation.x)
  var ySensor = Number(data.demo.rotation.y)
  var zSensor = Number(data.demo.rotation.z)

  zSensor = correct(zSensor, lastInput)

  // Drone starting position should be 0,0,0.
  if (!isRunning) {
    zero.x = -xSensor
    zero.y = -ySensor
    zero.z = -zSensor
  } else {
    xSensor += zero.x
    ySensor += zero.y
    zSensor += zero.z

    var xCorrect = pids.x.update(xSensor)
    var yCorrect = pids.y.update(ySensor)
    var zCorrect = pids.z.update(zSensor)

    set({ z: zCorrect })

    console.log('sensor: ' + zSensor.toFixed(2) + '  correct: ' + zCorrect.toFixed(2))

  }
})

