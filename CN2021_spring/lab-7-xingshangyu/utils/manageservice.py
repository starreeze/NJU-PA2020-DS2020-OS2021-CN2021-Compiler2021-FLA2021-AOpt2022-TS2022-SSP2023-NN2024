import sys
from datetime import datetime
from urllib.parse import urlunparse
from subprocess import Popen, PIPE


__all__ = ['startForCaching', 'startForDNS', 'startAll', 'terminateAll']


dsP, msP, rsP, csP = None, None, None, None


def log_info(msg):
    now = datetime.now().strftime("%Y/%m/%d-%H:%M:%S")
    print(f"{now}| [INFO] {msg}")


def log_error(msg):
    now = datetime.now().strftime("%Y/%m/%d-%H:%M:%S")
    print(f"{now}| [ERROR] {msg}", file=sys.stderr)


def startDNSServer(port=9999):
    cmd = ["python3", "runDNSServer.py", str(port)]
    p = Popen(cmd, stdout=PIPE, stderr=PIPE)
    if p.poll() is None:
        log_info("DNS server started")
        return p
    log_error("Cannot start DNS server")
    return None


def startMainServer(workdir, bind=None, port=8000):
    cmd = ["python3", "mainServer/mainServer.py", str(port), "-d", workdir]
    if bind is not None:
        cmd += ["-b", bind]
    p = Popen(cmd, stdout=PIPE, stderr=PIPE)
    if p.poll() is None:
        log_info("Main server started")
        return p
    log_error("Cannot start main server")
    return None


def startRPCServer(port=3322):
    cmd = ["python3", "utils/rpcServer.py", str(port)]
    p = Popen(cmd, stdout=PIPE, stderr=PIPE)
    if p.poll() is None:
        log_info("RPC server started")
        return p
    log_error("Cannot start RPC server")
    return None


def startCachingServer(mainServerAddr, port=1222, rpcAddr=None):
    cmd = ["python3", "runCachingServer.py", mainServerAddr, str(port)]
    if rpcAddr is not None:
        cmd += ["-r", rpcAddr]
    p = Popen(cmd, stdout=PIPE, stderr=PIPE)
    if p.poll() is None:
        log_info("Caching server started")
        return p
    log_error("Cannot start caching server")
    return None


def startForCaching(
        mainIP="127.0.0.1", mainPort=8000, mainWorkDir="mainServer/",
        cacheIP="127.0.0.1", cachePort=1222,
        rpcIP="127.0.0.1", rpcPort=3322
    ):
    '''Start main server, RPC server and caching server'''
    global msP, rsP, csP
    msP = startMainServer(workdir=mainWorkDir, bind=mainIP, port=mainPort)
    rsP = startRPCServer(port=rpcPort)
    mainAddr = f"{mainIP}:{mainPort}"
    rpcAddr = f"{rpcIP}:{rpcPort}"
    csP = startCachingServer(mainAddr, port=cachePort, rpcAddr=rpcAddr)
    return msP is not None and rsP is not None and csP is not None


def startForDNS(dnsIP="127.0.0.1", port=9999):
    '''Start DNS server'''
    global dsP
    dsP = startDNSServer(port=port)
    return dsP is not None


def startAll(
        dnsIP="127.0.0.1", dnsPort=9999,
        mainIP="127.0.0.1", mainPort=8000, mainWorkDir="mainServer/",
        cacheIP="127.0.0.1", cachePort=1222,
        rpcIP="127.0.0.1", rpcPort=3322
    ):
    '''Start DNS server, main server, RPC server and caching server'''
    global dsP, msP, rsP, csP
    dsP = startDNSServer(dnsPort)
    msP = startMainServer(workdir=mainWorkDir, bind=mainIP, port=mainPort)
    rsP = startRPCServer(port=rpcPort)
    mainAddr = f"{mainIP}:{mainPort}"
    rpcAddr = f"{rpcIP}:{rpcPort}"
    csP = startCachingServer(mainAddr, port=cachePort, rpcAddr=rpcAddr)
    return dsP is not None \
        and msP is not None \
        and rsP is not None \
        and csP is not None


def terminateAll():
    global dsP, msP, rsP, csP
    if dsP is not None:
        dsP.terminate()
        log_info("DNS server terminated")
    if csP is not None:
        csP.terminate()
        log_info("Caching server terminated")
    if rsP is not None:
        rsP.terminate()
        log_info("PRC server terminated")
    if msP is not None:
        msP.terminate()
        log_info("Main server terminated")
