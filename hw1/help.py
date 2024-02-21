import heapq
class NormalQueue:
    def __init__(self):
        self.items = []

    def enqueue(self, item):
        self.items.append(item)

    def dequeue(self):
        if self.is_empty():
            raise IndexError("dequeue from an empty queue")
        return self.items.pop(0)

    def front(self):
        if self.is_empty():
            raise IndexError("front from an empty queue")
        return self.items[0]

    def is_empty(self):
        return len(self.items) == 0

    def size(self):
        return len(self.items)

    def print_elements(self):
        for item in self.items:
            print(f'Process {item[0]} | Time Needed: {item[1]}',end=' ]')
    def printR_elements(self):
        return [i[0] for i in self.items]
class PriorityQueue:
    def __init__(self):
        self.heap = []

    def push(self, item, priority):
        heapq.heappush(self.heap, (priority, item))

    def pop(self):
        if self.is_empty():
            raise IndexError("pop from an empty priority queue")
        priority, item = heapq.heappop(self.heap)
        return priority, item

    def top(self):
        if self.is_empty():
            raise IndexError("top from an empty priority queue")
        return self.heap[0]

    def is_empty(self):
        return len(self.heap) == 0 

    def print_elements(self):
        for priority, item in self.heap:
            print(f" Process: {item[0]}| Command: {item[1]}| Time:{priority}",end=" ] ")
        
def print_processt(table):
    print(f'{"Process #"} | {"Start Time"}  | {"Start Line #"} | {"End Line #"} | {"Current Line"} | {"State"}  ')
    for j,i in enumerate(table):
        if(len(table[j])>4):
            print(f' {i[0]:3d} {i[1]:12d} {i[2]:12d}  {i[3]:12d} {i[4]:12d}            {i[5]}  ')
        else:
            print(f' {i[0]:3d} {i[1]:12d} {i[2]:12d}  {i[3]:12d}  ')
def readvalues():
    input_table = []
    with open('/home/brohudev/treasurechest/00_College/02_Sophomore_Year_Archive/spring/OS (cosc 3360)/hw/hw1/in.txt','r') as in_file:
        s= in_file.readline()
        while(s):
            s=s.strip()
            if(not len(s)): break
            s=[i for i in s.split(' ') if len(i)]
            s[1]=int(s[1])
            input_table.append(s)
            s=in_file.readline()
    return input_table
def process_table():
    b_size = 0
    values = readvalues()
    process_t =[]
    num_process=0
    for j,i in enumerate(values):
        if(i[0]=='BSIZE'):
            b_size=i[1]
        if(not len(process_t)):
            if(i[0]=='START'):
                process_t.append([num_process,int(i[1]),j])
                num_process+=1
        else:
            if(i[0]=='START'):
                process_t[num_process-1].append(j-1)
                process_t.append([num_process,int(i[1]),j])
                num_process+=1
            
            elif((j+1)==len(values)):
                process_t[num_process-1].append(j)
    print(f'{"Line#"}  {"Operation"} \n')
    for i in values:
        space =10
        left = space-len(i[0])
        print(f'{i[0]}{left*" "}{i[1]}')
    return process_t,b_size 
def has_arrived(Clock_Time,PID):
    global emptyCPU
    command,value = input_table[table_process[PID][4]]
    #print(command,value)
    if emptyCPU: #add it to the queue and add the completion time
        emptyCPU = False
        mainQueue.push([PID,'CORE'],value+Clock_Time)
        table_process[PID][5]='Running'
    else: #the cpu must be full
        ReadyQueue.enqueue([PID,value])
        table_process[PID][5]='Ready'
def finished_core(Clock_Time,PID):
    global emptyCPU
    if not ReadyQueue.is_empty():
        top = ReadyQueue.dequeue()
        mainQueue.push([top[0],'CORE'],top[1]+Clock_Time)
        table_process[top[0]][5]='Running'
    else:
        emptyCPU = True 
    #go to the next request
    if(table_process[PID][3]>=table_process[PID][4]+1):
        table_process[PID][4]+=1 
        Request_Sort(Clock_Time,PID)
    else:
        completed(Clock_Time,PID)
def finished_sdd(Clock_Time,PID):
    global EmptySSD
    if not SSDQueue.is_empty():
        top = SSDQueue.dequeue()
        mainQueue.push([top[0],'WRITE'],0.1+Clock_Time)
        table_process[top[0]][5]='Blocked'
    else:
        EmptySSD = True 
    #go to the next request
    if(table_process[PID][3]>=table_process[PID][4]+1):
        table_process[PID][4]+=1 
        Request_Sort(Clock_Time,PID)
    else:
        completed(Clock_Time,PID)
def completed(Clock_Time,PID):
    print(f'The process {PID} has terminated')
    table_process[PID][5]='Terminated'
def Request_Sort(Clock_Time,PID):

    global EmptySSD
    global emptyCPU
    Request=input_table[table_process[PID][4]]
    if(Request[0]=='WRITE'):
        if EmptySSD: #the SSD is empty 
            EmptySSD = False
            mainQueue.push([PID,Request[0]],Clock_Time+0.1)
            table_process[PID][5]='Blocked'
        else: #the SSD must be full
            SSDQueue.enqueue([PID,Request[1]])
            table_process[PID][5]='Blocked'
    if(Request[0]=='CORE'):
        if emptyCPU: #the CPU is empty 
            emptyCPU = False
            mainQueue.push([PID,Request[0]],Clock_Time+Request[1])
            table_process[PID][5]='Running'
        else: #the SSD must be full
            ReadyQueue.enqueue([PID,Request[1]])
            table_process[PID][5]='Ready'
def print_visual(x,clock,R,S,MQ,EC,ES):
    print('MainQueue : ',end=" ")
    if(not MQ.is_empty()):
        MQ.print_elements()
    else:
        print('[]',end=" ")
    print()
    print('ReadyQueue : ',end=" ")
    if(not R.is_empty()):
        R.print_elements()
    else:
        print('[]',end=" ")
    print()
    print('SSDQueue : ',end=" ")
    if(not S.is_empty()):
        S.print_elements()
    else:
        print('[]',end=" ")
    print()

    z=[i[-1] for i in x if str(i[-1]).isalpha()]
    for i,e in enumerate(z):
        print(f'Process {i} is {e}')
    # Calculate the length of the first string
    count1 = len('[A]' if EC else '[X]') + len(' RQ: ')+ len(' RQ: ')+len(str(S.printR_elements())) - 2

    # Calculate the length of the second string
    count2 = len('[A]' if EC else '[X]') + + len(' RQ: ')+ len(' RQ: ')+len(str(R.printR_elements()))- 2

    # Find the maximum length between the two strings
    maxie = max(count1, count2)
    #print(count1,count2)
    if(count1>count2):
        print(f" CP: {'[A]' if EC else '[X]'}{'-'*0}> SQ: {S.printR_elements()}")
        for i in range(2):
            print(f" |{maxie*' '}|")
        print(f" RQ: {R.printR_elements()}<{'-'*(count1-count2)} SD: {'[A]' if ES else '[X]'}")
    elif (count1< count2):
        print(f" CP: {'[A]' if EC else '[X]'}{'-'*(count2 -count1)}> SQ: {S.printR_elements()}")
        for i in range(2):
            print(f" |{(maxie)*' '}|")
        print(f" RQ: {R.printR_elements()}<{'-'*0} SD: {'[A]' if ES else '[X]'}")
    else:
        print(f" CP: {'[A]' if EC else '[X]'}{'-'*(0)}> SQ: {S.printR_elements()}")
        for i in range(2):
            print(f" |{maxie*' '}|")
        print(f" RQ: {R.printR_elements()}<{'-'*(0)} SD: {'[A]' if ES else '[X]'}")

   
#Create the Tables         
table_process,BSIZE=process_table()
input_table = readvalues()

#Create the mainQueue to Load Process 
mainQueue = PriorityQueue()
#Create the Queues for the CPU and SSD
ReadyQueue, SSDQueue = NormalQueue(),NormalQueue()
#Create the booleans for the 
emptyCPU,EmptySSD = True,True

#Loads all the Arrival Times in 
for i in range(len(table_process)):
    
    Time,ProcessID = table_process[i][1],table_process[i][0]
    mainQueue.push([ProcessID,'START'],Time) #The Time of the event,The processID 
    
Clock_Time = 0
print_visual(table_process,Clock_Time,ReadyQueue,SSDQueue,mainQueue,emptyCPU,EmptySSD)
while(not mainQueue.is_empty()):
    #Print the Process Table 
    print('-'*130)
    
    element_popped = mainQueue.pop()
    #print(element_popped)
    Time,ProcessID,Command = element_popped[0],element_popped[1][0],element_popped[1][1]
    Clock_Time = Time #set the Clock_Time
    if(Command=='START'):
        #print(f'Process {ProcessID} has arrived')
        table_process[ProcessID].append(table_process[ProcessID][2]) #Add the start line 
        table_process[ProcessID].append('Arrived')
        #get the next command (should be core)
        table_process[ProcessID][4]+=1 
        has_arrived(Clock_Time,ProcessID)
    if(Command=='CORE'):
        finished_core(Clock_Time,ProcessID)
    if(Command=='WRITE'):
        finished_sdd(Clock_Time,ProcessID)
    print()
    print_processt(table_process)
    print(f'The clock time is: {Clock_Time} ')
    print_visual(table_process,Clock_Time,ReadyQueue,SSDQueue,mainQueue,emptyCPU,EmptySSD)