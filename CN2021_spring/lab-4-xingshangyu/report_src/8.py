class ArpThread(threading.Thread):
    def __init__(self, queue: ArpQueue):
        threading.Thread.__init__(self, daemon=True)
        self.arpQueue = queue
    
    def run(self):
        while True:
            self.arpQueue.check_timeout()
            sleep(0.1)