/**
 * Copyright (c) dushin.net
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of dushin.net nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY dushin.net ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dushin.net BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package net.dushin.rabbitmq;

import java.io.IOException;

import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.MessageProperties;

public class Sender extends RabbitMQClient
{
    private final Connection connection;
    private final Channel channel;
    private final String exchangeName;
    private final String queueName;
    private final boolean isDurable;
    private static final java.util.Random generator = new java.util.Random();
    
    
    public Sender(
        final String hostname,
        final String exchangeName,
        final String queueName,
        final boolean isDurable
    ) throws IOException {
        super(hostname);
        this.connection = connectionFactory.newConnection();
        this.channel = connection.createChannel();
        channel.queueDeclare(queueName, isDurable, false, false, null);
        this.queueName = queueName;
        this.exchangeName = exchangeName;
        this.isDurable = isDurable;
    }
    
    
    public void send(final byte[] data) throws IOException {
        channel.basicPublish(exchangeName, queueName, isDurable ? MessageProperties.PERSISTENT_BASIC : null, data);
    }
    
    
    public void stop() throws IOException {
        channel.close();
        connection.close();
    }

    private static byte[][] DATA;
    
    private static void help() {
        System.out.println(
                "Syntax: " + Sender.class.getName() + '\n'
                + "\t--hostname <string> (default: \"localhost\")\n"
                + "\t--exchangeName <string> (default: \"\")\n"
                + "\t--queueName <string> (default: \"test\")\n"
                + "\t--numMessages <int> (default: 1)\n"
                + "\t--messageSize <int> (default: 1024)\n"
                + "\t--durable (default: FALSE)\n"
            );
    }

    public static void main(final String[] argv) throws IOException {
        generator.setSeed(System.currentTimeMillis());
        String hostname = "localhost";
        String exchangeName = "";
        String queueName = "test";
        Integer numMessages = 1;
        Integer messageSize = 1024;
        Boolean persistent = Boolean.FALSE;
        Boolean verbose = Boolean.FALSE;
        for (int i = 0;  i < argv.length;) {
            final String arg = argv[i];
            if (arg.equals("--hostname")) {
                hostname = nextString(argv, ++i);
                ++i;
            } else if (arg.equals("--exchangeName")) {
            	exchangeName = nextString(argv, ++i);
                ++i;
            } else if (arg.equals("--queueName")) {
            	queueName = nextString(argv, ++i);
                ++i;
            } else if (arg.equals("--numMessages")) {
            	numMessages = nextInt(argv, ++i);
                ++i;
            } else if (arg.equals("--messageSize")) {
            	messageSize = nextInt(argv, ++i);
                ++i;
            } else if (arg.equals("--durable")) {
            	persistent = Boolean.TRUE;
                ++i;
            } else if (arg.equals("--verbose")) {
            	verbose = Boolean.TRUE;
                ++i;
            } else if (arg.equals("--help")) {
            	help();
                System.exit(0);
            } else {
            	help();
                System.exit(1);
            }
        }
        final byte[] data = randomData(messageSize);
        if (verbose) {
            System.out.println("Sending " + numMessages + " message(s)...");
        }
     	int sent = 0;
        final Sender sender = new Sender(hostname, exchangeName, queueName, persistent);
        for (int i = 0;  i < numMessages;  ++i) {
            try {
                sender.send(data);
            } catch (IOException e) {
                e.printStackTrace(); 
            }
            ++sent;
            if (verbose) {
            	if (sent % 10000 == 0) {
            		System.out.print('.');
            	}
            }
        }
        if (verbose) {
        	System.out.println("Sent " + numMessages + " message(s).");
        }
        sender.stop();
    }

    private static byte[] randomData(Integer messageSize) {
        final byte[] bytes = new byte[messageSize];
        generator.nextBytes(bytes);
        return bytes;
    }
}
