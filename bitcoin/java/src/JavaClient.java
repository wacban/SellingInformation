import bitcoin_utils.*;

import org.apache.thrift.TException;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;

public class JavaClient {
  public static void main(String [] args) {
    try {
      TTransport transport;
      transport = new TSocket("localhost", 9090);
      transport.open();

      TProtocol protocol = new  TBinaryProtocol(transport);
      BitcoinUtils.Client client = new BitcoinUtils.Client(protocol);

      perform(client);

      transport.close();
    } catch (TException x) {
      x.printStackTrace();
    } 
  }

  private static void perform(BitcoinUtils.Client client) throws TException
  {
    client.ping();
    System.out.println("ping()");
  }
}
