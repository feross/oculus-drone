var arDrone = require('ar-drone')
var net = require('net')
var split = require('split')
var through = require('through')
var tty = require('tty')

process.stdin.setRawMode(true)

process.on('uncaughtException', function () {
  drone.land()
  setTimeout(function () {
    process.exit(0)
  }, 200)
})

var drone = arDrone.createClient()

// var client = net.connect({
//   port: 12345
// })
// .on('error', function () { console.log('error')})

// client
//   .pipe(split())
//   .pipe(through(function (line) {
//     var arr = line.split(',')
//     console.log(arr)
//     var x = -arr[0], rot = -arr[1], y = -arr[2]
//     set('x', x / Math.PI)
//     set('rot', rot / Math.PI)
//     set('y', y / Math.PI)
//   }))


drone.disableEmergency()
drone.stop() // Stop command drone was executing before batt died

var client = net.connect({
  port: 12345
})
.on('error', function () { console.log('error')})

client
  .pipe(split())
  .pipe(through(function (line) {
    var arr = line.split(',')
    var x = Number(-arr[0]), rot = Number(-arr[1]), y = Number(-arr[2])
    set('x', x)
    set('rot', rot)
    set('y', y)

    // Gestures:
    //   DOWN to takeoff/land
    //   UP to flip
    if (x < -1)
      flip()
    if (x > 1) {
      if (inAir) {
        land()
      } else {
        takeoff()
      }
    }
  }))
drone.on('batteryChange', function (num) {
  console.log('battery: ' + num)
})

enableKeyboardControl()

var params = { x: 0, y: 0, z: 0, rot: 0 }

function set (param, val) {
  if (val > 1)
    params[param] = 1
  else if (val < -1)
    params[param] = -1
  else
    params[param] = val

  if (param === 'x')
    drone.front(params[param])
  else if (param === 'y')
    drone.right(params[param])
  else if (param === 'z')
    drone.up(params[param])
  else if (param === 'rot')
    drone.clockwise(params[param])
  else
    console.error('Invalid param to `set`')
}

function reset () {
  ['x', 'y', 'z', 'rot'].forEach(function (param) {
    set(param, 0)
  })
}

function enableKeyboardControl () {
  process.stdin.on('data', function(chunk) {
    var key = chunk.toString()
    var keyBuf = chunk.toJSON()

    console.log(key)
    console.log(keyBuf)

    if (Array.isArray(keyBuf)) {
      var UP = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 65)
      var DOWN = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 66)
      var RIGHT = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 67)
      var LEFT = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 68)
    }

    if (key === 'w') {
      set('x', speed) // FORWARD

    } else if (key === 's') {
      set('x', -speed) // BACK

    } else if (key === 'd') {
      set('y', speed) // RIGHT

    } else if (key === 'a') {
      set('y', -speed) // LEFT

    } else if (UP) {
      set('z', speed) // UP

    } else if (DOWN) {
      set('z', -speed) // DOWN

    } else if (LEFT) {
      set('rot', -speed) // ROTATE COUNTERCLOCKWISE

    } else if (RIGHT) {
      set('rot', speed) // ROTATE CLOCKWISE

    } else if (key === 'e') {
      drone.disableEmergency()

    } else if (key === 't') {
      drone.disableEmergency()
      drone.stop()
      drone.takeoff()

    } else if (key === 'l') {
      drone.land()

    } else if (key === 'k') {
      drone.land()
      setTimeout(function () {
        process.exit(0)
      }, 200)

    } else if (key === 'q') {
      drone.stop()

    } else if (keyBuf[0] === 32) {
      drone.animate('flipAhead', 500)

      drone
        .after(750, function () {
          drone.down(1)
        })
        .after(200, function () {
          drone.down(0)
        })

    }

  })
}

// drone.on('navdata', function (data) {
//   console.log(data)
// })