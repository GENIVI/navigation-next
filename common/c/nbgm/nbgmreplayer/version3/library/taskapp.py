from task import *
from nbgm import NBGMDevice

class TaskProducer(Thread):

    '''TaskProducer thread is responsible for reading tasks from a log file and
putting them into the queue continually.
     This class uses a reader to parse files.
     '''
    def __init__(self, task_reader, task_queue, tname = 'TaskProducer'):
        assert(task_reader is not None and task_queue is not None)
        Thread.__init__(self, name = tname)
        self.__task_reader = task_reader
        self.__task_queue  = task_queue
        self.__stop_flag   = False

    def stop(self):
        self.__task_reader.stop()
        self.__stop_flag = True

    def run(self):
        self.__stop_flag  = False
        self.__task_reader.reset()
        if not self.__task_reader.initialize():
            logging.error('TaskProducer initialize failed!')
            return
        logging.info('Thread started...')
        while self.__task_reader.hasMore() and not self.__stop_flag:
            tasks = self.__task_reader.readTasks()
            for task in tasks:
                again = True
                while again and not self.__stop_flag:
                    try:
                        self.__task_queue.put(task, False, 0.1)
                    except Exception:
                        again = True
                    else:
                        again = False
        if not self.__task_reader.deinitialize():
            logging.error('TaskProducer deinitialize failed!')
            return
        logging.info('Thread exited...')


class TaskConsumer(Thread):

    '''TaskConsumer thread is responsible for picking a task form the queue continually.
After that, TaskConsumer will excute the task by some scheduling policy, which is specified by the dealer.
    This thread can be paused, stopped and restarted.
    '''
    def __init__(self, task_dealer, task_queue, tname = 'TaskConsumer'):
        assert(task_dealer is not None and task_queue is not None)
        Thread.__init__(self, name = tname)
        self.__task_dealer = task_dealer
        self.__task_queue  = task_queue
        self.__stop_flag   = False

    def stop(self): 
        self.__stop_flag = True
        self.__task_queue.queue.clear()
        self.__task_queue.put(Task.genTerminator())
        self.__task_dealer.stop()

    def pause(self, pause_flag):
        self.__task_dealer.pause(pause_flag)

    def run(self):
        self.__stop_flag = False
        self.__task_dealer.reset()
        if not self.__task_dealer.initialize():
            logging.error('TaskConsumer initialize failed!')
            return
        logging.info('Thread started...')
        while not self.__stop_flag:
            task = self.__task_queue.get()
            if task.isTerminator():
                self.__task_queue.task_done()
                break
            self.__task_dealer.scheduleTasks(task)
            self.__task_queue.task_done()
        if not self.__task_dealer.deinitialize():
            logging.error('TaskConsumer deinitialize failed!')
            return
        logging.info('Thread exited...')

class TaskApp:

    '''The TaskApp class is the base for creating task replay applications.
Think of it as your main entry point into the run loop.
    In most cases, you create an instance of this class and then call the build method,
when you are ready to start the application's lifecycle, you call your instance's run method.
    '''
    version = 1.0

    def __init__(self, device, task_reader, task_runner):
        assert(task_reader  and task_runner and device )
        self.__queue       = Queue(4096)
        self.__task_reader = task_reader
        self.__task_dealer = task_runner
        self.__producer    = None
        self.__consumer    = None
        self.__device      = device
        self.__pause_flag  = False

    def pause(self):
        '''Pause or resume the replay
        '''
        self.__pause_flag = not self.__pause_flag
        self.__consumer.pause(self.__pause_flag)
        if self.__pause_flag:
            logging.info('Replay paused..')
        else:
            logging.info('Repaly resumed...')


    def stop(self):
        '''Stop the replay
        '''
        pass


    def restart(self):
        '''Restart the repaly
        '''
        pass

    def run(self):
        logging.info('App started...')
        # create device
        logging.info('Device created...')
        success = self.__device.create()
        if not success:
            logging.error('Device create failed! App exited...')
            return

        self.__producer = TaskProducer(self.__task_reader, self.__queue)
        self.__consumer = TaskConsumer(self.__task_dealer, self.__queue)
        self.__producer.start()
        self.__consumer.start()

        # register key events
        self.__device.registerKeyEvent('p', self.pause)
        self.__device.registerKeyEvent('s', self.stop)
        self.__device.registerKeyEvent('r', self.restart)

        # start the mainloop
        self.__device.startMainLoop()
        self.__producer.stop()
        self.__consumer.stop()
        self.__producer.join()
        self.__consumer.join()

        # destroy device
        logging.info('Device destroyed...')
        self.__device.destroy()
        logging.info('App exited...')