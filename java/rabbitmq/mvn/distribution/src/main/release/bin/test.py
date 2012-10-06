#!/usr/bin/env python
#
# Copyright (c) 2012 RSA The Security Division of EMC
#

import optparse
import os
import subprocess
import sys
import threading
import time

#print sys.argv[0]

parser = optparse.OptionParser()
parser.add_option(
    "--hostname",
    metavar="HOST",
    type="string",
    default="localhost",
    help="AMQP Broker host (default: \"localhost\")"
)
parser.add_option(
    "--queueName",
    metavar="QUEUE_NAME",
    type="string",
    default="test",
    help="Queue name (default: \"test\")"
)
parser.add_option(
    "--numQueues",
    metavar="NUM_QUEUES",
    type="int",
    default=1,
    help="number of queues to run (default: 1)"
)
parser.add_option(
    "--numMessages",
    metavar="NUM_MESSAGES",
    type="int",
    default=1,
    help="number of messages to process (default: 1)"
)
parser.add_option(
    "--messageSize",
    metavar="MESSAGE_SIZE",
    type="int",
    default=1024,
    help="message size in bytes (default: 1024)"
)
parser.add_option(
    "--durable",
    action="store_true",
    default=False,
    dest="durable",
    help="whether the queue/messages should be durable/persistent (false by default)"
)
parser.add_option(
    "--verbose",
    action="store_true",
    default=False,
    dest="verbose",
    help="run in verbose mode (false by default)"
)

(options, args) = parser.parse_args()

def str(v) :
    return "%s" % v


def send_call(queueName, options) :
    try :
        args = [
            "sender",
            "--hostname", options.hostname,
            "--queueName", queueName,
            "--numMessages", str(options.numMessages),
            "--messageSize", str(options.messageSize)
        ]
        if (options.durable) :
            args.append("--durable")
        if (options.verbose) :
            args.append("--verbose")
        subprocess.call(args)
        #print "Send args", args
    except e :
        print e

def receive_call(queueName, options) :
    args = [
        "receiver",
        "--hostname", options.hostname,
        "--queueName", queueName,
        "--numMessages", str(options.numMessages)
    ]
    if (options.durable) :
        args.append("--durable")
    if (options.verbose) :
        args.append("--verbose")
    #print "Receive args", args
    subprocess.call(args)

def run_test(i, options) :
    start = time.time()
    queueName = "%s-%s" % (options.queueName, i)
    t = threading.Thread(name="send", target=send_call, args=(queueName, options))
    t.start()
    receive_call(queueName, options)
    elapsed = time.time() - start
    print "Received %s messages in %s seconds: %s MPS" % (options.numMessages, elapsed, options.numMessages/elapsed if elapsed > 0 else "inf")



def run(options) :
    threads = []
    start = time.time()
    for i in range(options.numQueues) :
        t = threading.Thread(name="run_test", target=run_test, args=(i, options))
        t.start()
        threads.append(t)
    for t in threads :
        t.join()
    elapsed = time.time() - start
    totalMessages = options.numMessages * options.numQueues
    print "Received %s messages in aggregate in %s seconds: %s MPS" % (totalMessages, elapsed, totalMessages/elapsed if elapsed > 0 else "inf")


run(options)
