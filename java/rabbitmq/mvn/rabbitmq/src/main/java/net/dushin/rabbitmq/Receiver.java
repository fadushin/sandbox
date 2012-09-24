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
import java.util.Iterator;

import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConsumerCancelledException;
import com.rabbitmq.client.QueueingConsumer;
import com.rabbitmq.client.ShutdownSignalException;

public class Receiver extends RabbitMQClient implements Iterable<byte[]> {
    
    private final Connection connection;
    private final Channel channel;
    private final MessageIterator iterator;

    public Receiver(
            final String hostname,
            final String queueName,
            final boolean isDurable
        ) throws IOException {
        super(hostname);
        this.connection = connectionFactory.newConnection();
        this.channel = connection.createChannel();
        channel.queueDeclare(queueName, isDurable, false, false, null);
        this.iterator = new MessageIterator(queueName, channel);
    }
    
    public void stop() throws IOException {
        channel.close();
        connection.close();
    }

    @Override
    public Iterator<byte[]> iterator() {
        return iterator;
    }
    
    private class MessageIterator implements Iterator<byte[]> {
        
        private final QueueingConsumer consumer;
        private QueueingConsumer.Delivery delivery;
        
        MessageIterator(final String queueName, final Channel channel) throws IOException {
            consumer = new QueueingConsumer(channel);
            channel.basicConsume(queueName, true, consumer);
        }
        
        @Override
        public boolean hasNext() {
            try {
                delivery = consumer.nextDelivery();
                return delivery != null;
            } catch (ShutdownSignalException e) {
                return false;
            } catch (ConsumerCancelledException e) {
                return false;
            } catch (InterruptedException e) {
                return false;
            }
        }

        @Override
        public byte[] next() {
            final byte[] data = delivery.getBody();
            if (data == null) {
                throw new IllegalStateException();
            }
            return data;
        }

        @Override
        public void remove() {
            throw new RuntimeException();
        }
        
    }
    
    
    public static void main(final String[] argv) throws IOException {
        String hostname = "localhost";
        String queueName = "test";
        Integer numReceive = 1;
        Boolean durable = Boolean.FALSE;
        for (int i = 0;  i < argv.length;) {
            final String arg = argv[i];
            if (arg.equals("--hostname")) {
                hostname = nextString(argv, ++i);
                ++i;
            }
            if (arg.equals("--queueName")) {
                queueName = nextString(argv, ++i);
                ++i;
            }
            if (arg.equals("--numReceive")) {
            	numReceive = nextInt(argv, ++i);
                ++i;
            }
            if (arg.equals("--durable")) {
            	durable = Boolean.TRUE;
                ++i;
            }
            if (arg.equals("--help")) {
                System.out.println(
                    "Syntax: " + Sender.class.getName() + '\n'
                    + "\t--hostname <string> (default: \"localhost\")\n"
                    + "\t--queueName <string> (default: \"test\")\n"
                    + "\t--numReceive <int> (default: 1)\n"
                    + "\t--durable (default: FALSE)\n"
                );
                System.exit(0);
            }
        }
        final int expect = numReceive;
        final Receiver receiver = new Receiver(hostname, queueName, durable);
        long numReceived = 0;
        long bytesReceived = 0L;
        for (final byte[] data : receiver) {
            numReceived++;
            bytesReceived += data.length;
            if (numReceived % 10000 == 0) {
                System.out.print('.');
            }
            if (numReceived >= expect) {
            	System.out.println();
            	System.out.println("Received " + numReceived + " message(s) in " + bytesReceived + " byte(s).");
            	break;
            }
        }
        receiver.stop();
    }
}
