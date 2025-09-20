let janus = null;
let streaming = null;

const server = "ws://10.222.63.101:8188"; // WHY DOES IT KEEP ON USING THE REST API AHHHHHHHHHHHHHHHH

Janus.init({
  debug: "all",
  dependencies: Janus.useDefaultDependencies({ adapter: adapter}),
  callback: function () {
    if (!Janus.isWebrtcSupported()) {
      alert("No WebRTC support... bye!");
      return;
    }

    janus = new Janus({
      server: server,
      success: function () {
        janus.attach({
          plugin: "janus.plugin.streaming",
          success: function (pluginHandle) {
            streaming = pluginHandle;
            Janus.log("Plugin attached! (" + streaming.getPlugin() + ", id=" + streaming.getId() + ")");

            // Request list of streams
            streaming.send({ message: { request: "list" } });

            // Watch stream with ID 1
            const body = { request: "watch", id: 1 };
            streaming.send({ message: body });
          },
          onmessage: function (msg, jsep) {
            Janus.debug("Got a message", msg);

            if (jsep !== undefined && jsep !== null) {
              streaming.createAnswer({
                jsep: jsep,
                media: { audioSend: false, videoSend: false },
                success: function (jsep) {
                  const body = { request: "start" };
                  streaming.send({ message: body, jsep: jsep });
                },
                error: function (error) {
                  Janus.error("WebRTC error:", error);
                  alert("WebRTC error... " + error.message);
                }
              });
            }
          },
          onremotestream: function (stream) {
            Janus.attachMediaStream(document.getElementById("remotevideo"), stream);
          },
          oncleanup: function () {
            Janus.log("Cleaned up");
          }
        });
      },
      error: function (error) {
        Janus.error(error);
        alert(error);
      },
      destroyed: function () {
        window.location.reload();
      }
    });
  }
});
