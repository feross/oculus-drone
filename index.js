var arDrone = require('ar-drone')
var fs = require('fs')
var net = require('net')
var PaVEParser = require('./node_modules/ar-drone/lib/video/PaVEParser')
var split = require('split')
var through = require('through')

process.on('uncaughtException', function (err) {
  console.error(err)
  land()
  setTimeout(function () {
    process.exit(0)
  }, 200)
})

process.stdin.setRawMode(true)

var drone = arDrone.createClient()
var inAir = false
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

var params = { x: 0, y: 0, z: 0, rot: 0 }

setInterval(function () {
  console.log(params)
}, 1000)

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

function takeoff () {
  inAir = true
  drone.disableEmergency()
  drone.stop()
  drone.takeoff()
}

function land () {
  inAir = false
  drone.land()
}

function flip () {
  drone.animate('flipAhead', 500)

  drone
    .after(750, function () {
      drone.down(1)
    })
    .after(200, function () {
      drone.down(0)
    })
}

process.stdin.on('data', function(chunk) {
  var key = chunk.toString()
  var keyBuf = chunk.toJSON()

  console.log(key)

  if (key === 'e') {
    drone.disableEmergency()

  } else if (key === 't') {
    takeoff()

  } else if (key === 'l') {
    land()

  } else if (key === 'k') {
    console.log('KILL')
    land()
    setTimeout(function () {
      process.exit(0)
    }, 200)

  } else if (key === 'q') {
    drone.stop()

  } else if (keyBuf[0] === 32) {
    flip()
  }
})

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



// drone.on('navdata', function (data) {
//   console.log(data)
// })

