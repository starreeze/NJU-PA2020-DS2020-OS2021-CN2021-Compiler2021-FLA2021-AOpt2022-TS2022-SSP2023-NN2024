with open(dns_file, mode='r') as file:
    content = file.read()
    if(content[-1] == '\n'):
        content = content[:-1]
    for line in content.split('\n'):
        items = line.split(' ')
        for i in range(len(items)):
            if items[i][-1] == '.':
                items[i] = items[i][:-1]
        self.dns_table.append(items)