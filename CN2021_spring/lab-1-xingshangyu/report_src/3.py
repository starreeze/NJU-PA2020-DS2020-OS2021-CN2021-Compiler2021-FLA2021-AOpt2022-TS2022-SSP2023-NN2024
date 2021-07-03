# my testcase: a frame from client to hub should result in nothing happening
testpkt = new_packet(
    "30:00:00:00:00:02",
    "10:00:00:00:00:01",
    "192.168.1.100",
    "172.16.42.2"
)
s.expect(
    PacketInputEvent("eth0", testpkt, display=Ethernet),
    ("A frame from client to hub should result in nothing happening.")
)