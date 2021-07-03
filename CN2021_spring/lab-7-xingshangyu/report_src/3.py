def select_ip(self, lst, clientip):
	pc = IP_Utils.getIpLocation(str(clientip))
	if pc is None:
		return random.choice(lst)
	dist = math.inf
	for sip in lst:
		ps = IP_Utils.getIpLocation(sip)
		if ps is None:
			return random.choice(lst)
		tmp_dist = self.calc_distance(pc[1], pc[0], ps[1], ps[0])
		if tmp_dist < dist:
			res = sip
			dist = tmp_dist
	return res