package fi.zzz.overlook;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.VectorDrawable;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.PowerManager;
import android.os.RemoteException;
import android.support.v4.app.NotificationCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.graphics.drawable.DrawableCompat;
import android.util.Log;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.Vector;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import static android.content.Intent.ACTION_MAIN;


public class AppService extends Service {
    public static AppService last;

    private static final String TAG = "Overlook";


    public static int setup_port = 17001;
    public static String setup_server = "overlook.zzz.fi";

    private int user_id = -1;
    private byte[] pass = new byte[8];
    public boolean is_registered = false;
    private int login_fails = 0;

    public Bitmap prev_set_image = null;
    public String user_name;
    public Socket sock;
    public double prev_lon = 0, prev_lat = 0, prev_alt = 0;
    public long login_id = 0;
    public int age = 0;
    public boolean is_logged_in = false;
    public DataInputStream input;
    public DataOutputStream output;
    public Lock call_lock = new ReentrantLock();
    public Lock lock = new ReentrantLock();

    public List<QuotesData> quote_values = new Vector<>();
    public List<Double> opens = new Vector<>();
    public List<Double> lows = new Vector<>();
    public List<Double> highs = new Vector<>();
    public List<String> symbols;
    public List<String> tflist;
    public Map<Integer, String> tfs;
    public List<String> sent_pairs;
    public Set<String> currencies;
    public List<String> sent_currencies;
    public double equity, balance, freemargin;
    public List<Order> open_orders;
    public List<Order> history_orders;
    public List<CalEvent> cal_events;
    public List<Event> events = new Vector<>();
    public List<SentimentSnapshot> senthist_list = new Vector<>();
    public List<EventError> errors = new Vector<>();


    public void setupFinish(String server, int port) {
        setup_server = server;
        setup_port = port;
        storeThis();
        startThread();
    }

    public void settingsFinish(String server, int port) {
        this.setup_server = server;
        this.setup_port = port;
        applySettings();
    }



    // Try to keep process alive
    PowerManager powerManager;
    PowerManager.WakeLock wakeLock;
    private final static int INTERVAL = 1000 * 60 * 2; //2 minutes
    Handler mHandler = new Handler();
    Runnable mHandlerTask = new Runnable()
    {
        @Override
        public void run() {
            dummyTask();
            mHandler.postDelayed(mHandlerTask, 1000*10);
        }
    };




    public AppService() {
        last = this;

        mHandlerTask.run();
    }

    void dummyTask() {
        Log.i(TAG, "Dummy task");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }


    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind done");
        return null;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return false;
    }


    @Override
    public void onCreate() {
        last = this;

        Log.i(TAG, "SERVICE STARTING");

        loadThis();

        startServiceWithNotification();
        startWakeLock();

        handleRegister();

        sendPostRefreshGui();

        super.onCreate();
    }

    void startServiceWithNotification() {
        Intent notificationIntent = new Intent(getApplicationContext(), AppService.class);
        notificationIntent.setAction(ACTION_MAIN);  // A string containing the action name
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent contentPendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, PendingIntent.FLAG_CANCEL_CURRENT);

        Bitmap icon = getBitmapFromVectorDrawable(R.drawable.ic_overlook);

        Notification notification = new NotificationCompat.Builder(this)
                .setContentTitle(getResources().getString(R.string.app_name))
                .setTicker(getResources().getString(R.string.overlook))
                .setContentText(getResources().getString(R.string.content_text))
                .setSmallIcon(R.drawable.ic_forum_black_24dp)
                .setLargeIcon(Bitmap.createScaledBitmap(icon, 128, 128, false))
                .setContentIntent(contentPendingIntent)
                .setOngoing(true)
//                .setDeleteIntent(contentPendingIntent)  // if needed
                .build();
        notification.flags = notification.flags | Notification.FLAG_NO_CLEAR;     // NO_CLEAR makes the notification stay when the user performs a "delete all" command
        startForeground(14125346, notification);
    }

    public Bitmap getBitmapFromVectorDrawable(int drawableId) {
        Drawable drawable = ContextCompat.getDrawable(getApplicationContext(), drawableId);
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            drawable = (DrawableCompat.wrap(drawable)).mutate();
        }

        Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(),
                drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);

        return bitmap;
    }

    void startWakeLock() {
        powerManager = (PowerManager)getApplicationContext().getSystemService(POWER_SERVICE);
        wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "Overlook::Service");
        wakeLock.acquire();
    }

    void handleRegister() {
        if (!is_registered) {
            sendStartOnboarding();
        }
        else {
            startThread();
        }
    }


    void NotifyNewMessage(int level, String message) {


        NotificationCompat.Builder builder =
                new NotificationCompat.Builder(this)
                        .setSmallIcon(R.drawable.ic_forum_black_24dp)
                        .setContentTitle(message)
                        .setContentInfo(message)
                        .setContentText(message);


        Uri alarmSound = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
        if (level == 1)
            alarmSound = Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.warning);
        else if (level == 2)
            alarmSound = Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.error);

        builder.setSound(alarmSound);

        Intent notificationIntent = new Intent(this, MainActivity.class);
        notificationIntent.setAction(Intent.ACTION_MAIN);

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        builder.setContentIntent(contentIntent);
        builder.setAutoCancel(true);
        builder.setLights(Color.BLUE, 500, 500);
        long[] pattern = {500,500,500,500,500,500,500,500,500};
        builder.setVibrate(pattern);
        builder.setStyle(new NotificationCompat.InboxStyle());
// Add as notification
        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        builder.setStyle(new NotificationCompat.InboxStyle());
        manager.notify(1, builder.build());
    }



    void startThread() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                connect();
                registerScript();
                loginScript();
                handleConnection();
            }
        };
        thread.start();
    }

    void applySettings() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                storeThis();
                setup();
            }
        };
        thread.start();
    }



    void writeBytes(DataOutputStream s, byte[] b) throws IOException {
        s.writeInt(b.length);
        s.write(b);
    }

    void writeString(DataOutputStream s, String str) throws IOException {
        writeBytes(s, str.getBytes());
    }

    void writeImage(DataOutputStream s, Bitmap bm) throws IOException {
        ByteArrayOutputStream baos= new ByteArrayOutputStream();
        bm.compress(Bitmap.CompressFormat.PNG,100, baos);
        byte [] b = baos.toByteArray();
        writeBytes(s, b);
    }

    byte[] readBytes(DataInputStream s) throws IOException {
        int len = s.readInt();
        if (len < 0 || len > 10000000) throw new IOException();
        byte[] b = new byte[len];
        s.read(b);
        return b;
    }

    String readString(DataInputStream s) throws IOException {
        return new String(readBytes(s));
    }

    byte[] readBytesSwap(DataInputStream s) throws IOException {
        int len = swap(s.readInt());
        if (len < 0 || len > 10000000) throw new IOException();
        byte[] b = new byte[len];
        s.read(b);
        return b;
    }

    String readStringSwap(DataInputStream s) throws IOException {
        return new String(readBytesSwap(s));
    }

    Bitmap readImage(DataInputStream s) throws IOException {
        byte[] b = readBytes(s);
        return BitmapFactory.decodeByteArray(b, 0, b.length);
    }

    void storeThis() {
        String cl_file = getApplicationContext().getFilesDir() + "/Client.bin";
        try {
            FileOutputStream fout = new FileOutputStream(cl_file );
            DataOutputStream out = new DataOutputStream(fout);
            out.writeInt(user_id);
            writeBytes(out, pass);
            out.writeBoolean(is_registered);
            writeString(out, setup_server);
            out.writeInt(setup_port);
            fout.close();
        }
        catch (IOException e) {
            Log.e(TAG, "Storing configuration failed");
        }
    }

    void loadThis() {
        String cl_file = getApplicationContext().getFilesDir() + "/Client.bin";
        try {
            FileInputStream fin = new FileInputStream(cl_file );
            DataInputStream in = new DataInputStream(fin);
            user_id = in.readInt();
            pass = readBytes(in);
            is_registered = in.readBoolean();
            setup_server = readString(in);
            setup_port = in.readInt();
            fin.close();
        }
        catch (IOException e) {
            Log.e(TAG, "Storing configuration failed");
        }
    }

    void disconnect() {
        if (sock != null) {
            try {
                sock.close();
            } catch (IOException e) {

            }
        }
        sock = null;
    }

    boolean connect() {
        if (sock == null || sock.isClosed()) {
            //sendPostSetTitle(getApplicationContext().getResources().getString(R.string.connecting));

            Log.i(TAG, "Connecting");

            try {
                Log.i(TAG, "Connecting " + setup_server + ":" + Integer.toString(setup_port));
                sock = new Socket(setup_server, setup_port);

                if (sock != null) {
                    input = new DataInputStream(this.sock.getInputStream());
                    output = new DataOutputStream(this.sock.getOutputStream());
                }

            }
            catch (UnknownHostException e1) {
                Log.w(TAG, "Couldn't resolve host");
                return false;
            }
            catch (IOException e) {
                Log.w(TAG, "Socket IO error");
                return false;
            }

            //sendPostSetTitle(getApplicationContext().getResources().getString(R.string.activity_main));
        }
        return true;
    }

    boolean registerScript() {
        if (!is_registered) {
            sendPostSetTitle(getApplicationContext().getResources().getString(R.string.registering));
            try {
                register();
                is_registered = true;
                is_logged_in = false;
                storeThis();
            }
            catch (Exc e) {
                return false;
            }
            sendPostSetTitle(getApplicationContext().getResources().getString(R.string.activity_main));
        }
        return true;
    }

    boolean loginScript() {
        if (!is_logged_in) {
            sendPostSetTitle(getApplicationContext().getResources().getString(R.string.logging_in));

            try {
                if (login()) {
                    login_fails = 0;
                }
                else {
                    login_fails++;
                    if (login_fails > 10)
                        is_registered = false;
                    throw new Exc("Login failed");
                }
                initValues();
                is_logged_in = true;
                sendPostRefreshGui();
            }
            catch (Exc e) {
                return false;
            }
            finally {
                sendPostSetTitle(getApplicationContext().getResources().getString(R.string.activity_main));
            }

            setup();
        }
        return true;
    }

    void setup() {
        sendPostSetTitle(getApplicationContext().getResources().getString(R.string.setupping));


        sendPostSetTitle(getApplicationContext().getResources().getString(R.string.activity_main));
    }

    void handleConnection() {
        sendPostSetTitle(getApplicationContext().getResources().getString(R.string.activity_main));
        Log.i(TAG, "Client connection running");
        int count = 0;

        while (!Thread.interrupted()) {

            try {
                while (!Thread.interrupted()) {
                    registerScript();
                    loginScript();

                    poll();
                    sleep(5000);
                    count++;
                }

                sock.close();
            }
            catch (Exc e) {
                Log.e(TAG, "Error: " + e.msg);
            }
            catch (IOException e) {
                Log.e(TAG, "Error: IOException");
            }
            catch (NullPointerException e) {

            }

            is_logged_in = false;

            try {sock.close();} catch (IOException e) {} catch (NullPointerException e) {}
            sock = null;
        }

        Log.i(TAG, "Client connection stopped");
    }

    int swap(int i) {
        return ByteBuffer.allocate(4)
                .order(ByteOrder.BIG_ENDIAN).putInt(i)
                .order(ByteOrder.LITTLE_ENDIAN).getInt(0);
    }

    public static double swapDouble(double x) {
        return ByteBuffer.allocate(8)
                .order(ByteOrder.BIG_ENDIAN).putDouble(x)
                .order(ByteOrder.LITTLE_ENDIAN).getDouble(0);
    }

    public static long swapLong(long x) {
        return ByteBuffer.allocate(8)
                .order(ByteOrder.BIG_ENDIAN).putLong(x)
                .order(ByteOrder.LITTLE_ENDIAN).getLong(0);
    }

    private final static char[] hexArray = "0123456789ABCDEF".toCharArray();
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    DataInputStream call(byte[] out_data) throws Exc {
        byte[] in_data;
        int r;

        call_lock.lock();

        disconnect();
        connect();

        if (sock == null) {
            call_lock.unlock();
            return new DataInputStream(new ByteArrayInputStream(new byte[0]));
        }


        try {
            sock.setSoTimeout(30000);

            output.writeInt(swap(out_data.length));
            output.write(out_data);
            //Log.i(TAG, bytesToHex(out_data));

            int in_size = swap(input.readInt());
            if (in_size <= 0 || in_size > 10000000)
                throw new SocketException();
            in_data = new byte[in_size];
            for (int i = 0; i < 100 && input.available() < in_size; i++) sleep(10);
            r = input.read(in_data);
            if (r != in_size)
                throw new IOException();
            //Log.i(TAG, bytesToHex(in_data));
        }
        catch (SocketException e) {
            throw new Exc("Call: Socket exception");
        }
        catch (EOFException e) {
            throw new Exc("Call: EOFException ");
        }
        catch (IOException e) {
            throw new Exc("Call: IOException");
        }
        finally {
            disconnect();
            call_lock.unlock();
        }
        return new DataInputStream(new ByteArrayInputStream(in_data));
    }

    void sleep(int ms) {
        try {
            Thread.sleep(ms);
        }
        catch (InterruptedException e) {}
    }

    int hash(byte[] s) {
        return memhash(s, s.length);
    }

    int memhash(byte[] ptr, int count) {
        int hash = 1234567890;
	    for (int i = 0; i < count; i++)
            hash = ((hash << 5) - hash) ^ (int)ptr[i];
        return hash;
    }

    void writeInt(ByteArrayOutputStream out, int i) {
        out.write((i >> 24) & 0xFF);
        out.write((i >> 16) & 0xFF);
        out.write((i >> 8) & 0xFF);
        out.write(i & 0xFF);
    }

    void register() throws Exc {
        try {
            ByteArrayOutputStream dout = new ByteArrayOutputStream();
            DataOutputStream out = new DataOutputStream(dout);

            out.writeInt(swap(10));

            DataInputStream in = call(dout.toByteArray());

            user_id = swap(in.readInt());
            byte[] pass_bytes = new byte[8];
            in.read(pass_bytes);
            pass = pass_bytes;

            Log.i(TAG, "Client " + Integer.toString(user_id) + " registered (pass " + new String(pass) + ")");
        }
        catch (IOException e) {
            throw new Exc("Register: IOException");
        }
    }

    boolean login() throws Exc {
        try {
            ByteArrayOutputStream dout = new ByteArrayOutputStream();
            DataOutputStream out = new DataOutputStream(dout);

            out.writeInt(swap(20));
            out.writeInt(swap(user_id));
            out.write(pass);

            DataInputStream in = call(dout.toByteArray());

            int ret = swap(in.readInt());
            if (ret == 1)
                return false;

            login_id = swapLong(in.readLong());

            Log.i(TAG, "Client " + Integer.toString(user_id) + " logged in (" + Integer.toString(user_id) + "," + new String(pass) + ") name: " + user_name);
            return true;
        }
        catch (IOException e) {
            throw new Exc("Register: IOException");
        }
    }
    boolean set(String key, byte[] value) throws Exc {
        try {
            ByteArrayOutputStream dout = new ByteArrayOutputStream();
            DataOutputStream out = new DataOutputStream(dout);

            out.writeInt(swap(30));

            out.writeLong(swapLong(login_id));

            out.writeInt(swap(key.length()));
            out.write(key.getBytes());
            out.writeInt(swap(value.length));
            out.write(value);

            DataInputStream in = call(dout.toByteArray());

            int ret = swap(in.readInt());
            if (ret == 1) {
                Log.e(TAG, "Client set " + key + " failed");
                return false;
            }
            Log.i(TAG, "Client set " + key);
            return true;
        }
        catch (IOException e) {
            throw new Exc("Register: IOException");
        }
    }

    byte[] get(String key) throws Exc {
        return get(key.getBytes());
    }

    byte[] get(byte[] key) throws Exc {
        try {
            ByteArrayOutputStream dout = new ByteArrayOutputStream();
            DataOutputStream out = new DataOutputStream(dout);

            out.writeInt(swap(40));

            out.writeLong(swapLong(login_id));

            out.writeInt(swap(key.length));
            out.write(key);

            DataInputStream in = call(dout.toByteArray());

            int value_len = swap(in.readInt());
            if (value_len < 0 || value_len > 10000000) {
                Log.e(TAG, "Get; Invalid length");
                return new byte[0];
            }
            byte[] value_bytes = new byte[value_len];
            in.read(value_bytes);

            Log.i(TAG, "Client get " + key);
            return value_bytes;
        }
        catch (IOException e) {
            throw new Exc("Register: IOException");
        }
    }

    int find(byte[] b, int chr) {
        for (int i = 0; i < b.length; i++) {
            if (b[i] == chr)
                return i;
        }
        return -1;
    }

    String midString(byte[] b, int begin, int length) {
        return new String(mid(b, begin, length));
    }

    byte[] mid(byte[] b, int begin, int length) {
        byte[] out = new byte[length];
        for(int i = 0; i < length; i++)
            out[i] = b[begin + i];
        return out;
    }

    void poll() throws Exc {

        lock.lock();

        try {
            ByteArrayOutputStream dout = new ByteArrayOutputStream();
            DataOutputStream out = new DataOutputStream(dout);

            out.writeInt(swap(50));

            out.writeLong(swapLong(login_id));

            DataInputStream in = call(dout.toByteArray());

            int max_polled_level = -1;
            String max_polled_msg = "";

            int count = swap(in.readInt());
            if (count < 0 || count >= 10000) {lock.unlock(); throw new Exc("Polling failed");}
            for(int i = 0; i < count; i++) {
                int message_len = swap(in.readInt());
                if (message_len < 0 ||message_len > 1000000) {
                    Log.e(TAG, "Invalid message");
                    break;
                }
                byte[] message_bytes = new byte[message_len];
                in.read(message_bytes);

                int j = find(message_bytes, ' ');
                if (j == -1) continue;
                String key = midString(message_bytes, 0, j);
                message_bytes = mid(message_bytes, j+1, message_bytes.length - j-1);


                Log.i(TAG, "Poll " + Integer.toString(i) + ": " + key);

                if (key.equals("info")) {
                    Event e = new Event();
                    e.msg = new String(message_bytes);
                    e.level = 0;
                    e.received = Calendar.getInstance().getTime();
                    if (max_polled_level < 0) {max_polled_level = 0; max_polled_msg = e.msg;}
                    events.add(e);
                    sendPostRefreshGui();
                }
                if (key.equals("warning")) {
                    Event e = new Event();
                    e.msg = new String(message_bytes);
                    e.level = 1;
                    e.received = Calendar.getInstance().getTime();
                    if (max_polled_level < 1) {max_polled_level = 1; max_polled_msg = e.msg;}
                    events.add(e);
                    sendPostRefreshGui();
                }
                if (key.equals("error")) {
                    Event e = new Event();
                    e.msg = new String(message_bytes);
                    e.level = 2;
                    e.received = Calendar.getInstance().getTime();
                    if (max_polled_level < 2) {max_polled_level = 2; max_polled_msg = e.msg;}
                    events.add(e);
                    sendPostRefreshGui();
                }
            }

            if (max_polled_level >= 0) {
                NotifyNewMessage(max_polled_level, max_polled_msg);
            }
        }
        catch (IOException e) {
            Log.e(TAG, "Poll: IOEXception");
        }
        catch (NullPointerException e) {
            Log.e(TAG, "Poll: NullPointerException");
        }
        finally {
            lock.unlock();
        }

    }

    void sendStartOnboarding() {
        MainActivity.last.postStartOnboarding();
    }

    void sendPostRefreshGui() {
        if (MainActivity.last != null)
            MainActivity.last.postRefreshGui();
    }

    void sendPostSetTitle(String title) {
        MainActivity.last.postSetTitle(title);
    }

    void initValues() {
        try {
            byte[] data = get("symtf");
            DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

            symbols = new Vector<String>();
            tflist = new Vector<String>();
            tfs = new HashMap<Integer, String>();
            sent_pairs = new Vector<String>();
            currencies = new HashSet<String>();
            sent_currencies = new Vector<String>();

            {
                int sym_count = swap(in.readInt());
                for (int i = 0; i < sym_count; i++) {
                    int sym_len = swap(in.readInt());
                    byte[] sym = new byte[sym_len];
                    in.read(sym);
                    symbols.add(new String(sym));
                }
            }

            {
                int sym_count = swap(in.readInt());
                for (int i = 0; i < sym_count; i++) {
                    int tf = swap(in.readInt());
                    int sym_len = swap(in.readInt());
                    byte[] sym = new byte[sym_len];
                    in.read(sym);
                    tfs.put(tf, new String(sym));
                    tflist.add(new String(sym));
                }
            }

            {
                int sym_count = swap(in.readInt());
                for (int i = 0; i < sym_count; i++) {
                    int sym_len = swap(in.readInt());
                    byte[] sym = new byte[sym_len];
                    in.read(sym);
                    currencies.add(new String(sym));
                }
            }

            {
                int sym_count = swap(in.readInt());
                for (int i = 0; i < sym_count; i++) {
                    int sym_len = swap(in.readInt());
                    byte[] sym = new byte[sym_len];
                    in.read(sym);
                    String s = new String(sym);
                    sent_pairs.add(s);
                }
            }

            {
                int sym_count = swap(in.readInt());
                for (int i = 0; i < sym_count; i++) {
                    int sym_len = swap(in.readInt());
                    byte[] sym = new byte[sym_len];
                    in.read(sym);
                    sent_currencies.add(new String(sym));
                }
            }

            MainActivity.last.initSentiment();
        }
        catch (Exc e) {}
        catch (IOException e) {}
    }


    void startRefreshQuotes() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<QuotesData> tmp = new Vector<>();

                try {
                    byte[] data = get("quotes");
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    int count = swap(in.readInt());
                    for (int i = 0; i < count; i++) {
                        QuotesData q = new QuotesData();

                        int sym_len = swap(in.readInt());
                        byte[] sym = new byte[sym_len];
                        in.read(sym);
                        q.symbol = new String(sym);

                        q.ask = swapDouble(in.readDouble());
                        q.bid = swapDouble(in.readDouble());
                        q.point = swapDouble(in.readDouble());

                        tmp.add(q);
                    }
                }
                catch (Exc e) {}
                catch (IOException e) {}

                quote_values = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.dataQuotes();
                    }
                });
            }
        };
        thread.start();
    }

    void startRefreshGraph(final int sym, final int tf) {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<QuotesData> tmp = new Vector<>();

                try {
                    byte[] data = get("graph," + Integer.toString(sym) + "," + Integer.toString(tf) + ",100");
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    List<Double> opens = new Vector<>();
                    List<Double> lows = new Vector<>();
                    List<Double> highs = new Vector<>();
                    int count = swap(in.readInt());
                    for (int i = 0; i < count; i++) {
                        double open = swapDouble(in.readDouble());
                        double low  = swapDouble(in.readDouble());
                        double high = swapDouble(in.readDouble());
                        opens.add(open);
                        lows.add(low);
                        highs.add(high);
                    }
                    AppService.last.highs = highs;
                    AppService.last.lows = lows;
                    AppService.last.opens = opens;
                }
                catch (Exc e) {}
                catch (IOException e) {}

                quote_values = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.dataGraph();
                    }
                });
            }
        };
        thread.start();
    }

    void startRefreshOrders() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<Order> tmp = new Vector<>();

                try {
                    byte[] data = get("openorders");
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    balance = swapDouble(in.readDouble());
                    equity = swapDouble(in.readDouble());
                    freemargin = swapDouble(in.readDouble());

                    int order_count = swap(in.readInt());
                    for(int i = 0; i < order_count; i++) {
                        Order o = new Order();
                        o.ticket = swap(in.readInt());
                        o.begin = new Date(swap(in.readInt()) * 1000L);
                        o.end = new Date(swap(in.readInt()) * 1000L);
                        o.end = null;
                        o.type = swap(in.readInt());
                        o.size = swapDouble(in.readDouble());
                        int sym_len = swap(in.readInt());
                        byte[] sym = new byte[sym_len];
                        in.read(sym);
                        o.symbol = new String(sym);
                        o.open = swapDouble(in.readDouble());
                        o.stoploss = swapDouble(in.readDouble());
                        o.takeprofit = swapDouble(in.readDouble());
                        o.close = swapDouble(in.readDouble());
                        o.commission = swapDouble(in.readDouble());
                        o.swap = swapDouble(in.readDouble());
                        o.profit = swapDouble(in.readDouble());

                        tmp.add(o);
                    }

                }
                catch (Exc e) {}
                catch (IOException e) {}

                open_orders = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.dataOrders();
                    }
                });
            }
        };
        thread.start();
    }

    void startRefreshHistory() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<Order> tmp = new Vector<>();

                try {
                    byte[] data = get("historyorders");
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    balance = swapDouble(in.readDouble());
                    equity = swapDouble(in.readDouble());
                    freemargin = swapDouble(in.readDouble());

                    int order_count = swap(in.readInt());
                    for(int i = 0; i < order_count; i++) {
                        Order o = new Order();
                        o.ticket = swap(in.readInt());
                        o.begin = new Date(swap(in.readInt()) * 1000L);
                        o.end = new Date(swap(in.readInt()) * 1000L);
                        o.type = swap(in.readInt());
                        o.size = swapDouble(in.readDouble());
                        int sym_len = swap(in.readInt());
                        byte[] sym = new byte[sym_len];
                        in.read(sym);
                        o.symbol = new String(sym);
                        o.open = swapDouble(in.readDouble());
                        o.stoploss = swapDouble(in.readDouble());
                        o.takeprofit = swapDouble(in.readDouble());
                        o.close = swapDouble(in.readDouble());
                        o.commission = swapDouble(in.readDouble());
                        o.swap = swapDouble(in.readDouble());
                        o.profit = swapDouble(in.readDouble());

                        tmp.add(o);
                    }

                }
                catch (Exc e) {}
                catch (IOException e) {}

                history_orders = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.dataHistoryOrders();
                    }
                });
            }
        };
        thread.start();
    }

    void startRefreshCalendar() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<CalEvent> tmp = new Vector<>();

                try {
                    byte[] data = get("calendar");
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    int ev_count = swap(in.readInt());
                    for(int i = 0; i < ev_count; i++) {
                        CalEvent e = new CalEvent();

                        e.id = swap(in.readInt());
                        e.timestamp = new Date(swap(in.readInt()) * 1000L);
                        e.impact = swap(in.readInt());
                        e.direction = swap(in.readInt());
                        e.title = readStringSwap(in);
                        e.unit = readStringSwap(in);
                        e.currency = readStringSwap(in);
                        e.forecast = readStringSwap(in);
                        e.previous = readStringSwap(in);
                        e.actual = readStringSwap(in);

                        tmp.add(e);
                    }

                }
                catch (Exc e) {}
                catch (IOException e) {}

                cal_events = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.dataCalendar();
                    }
                });
            }
        };
        thread.start();
    }

    void startRefreshSentimentHistory() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<SentimentSnapshot> tmp = new Vector<>();

                try {
                    byte[] data = get("senthist");
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    int sent_count = swap(in.readInt());
                    for(int i = 0; i < sent_count ; i++) {
                        SentimentSnapshot snap = new SentimentSnapshot();
                        snap.cur_pres = new Vector<Integer>();
                        snap.pair_pres = new Vector<Integer>();

                        int cur_count = swap(in.readInt());
                        for(int j = 0; j < cur_count; j++)
                            snap.cur_pres.add(swap(in.readInt()));

                        int pair_count = swap(in.readInt());
                        for(int j = 0; j < pair_count ; j++)
                            snap.pair_pres.add(swap(in.readInt()));

                        snap.comment = readStringSwap(in);
                        snap.added = new Date(swap(in.readInt()) * 1000L);

                        snap.fmlevel = swapDouble(in.readDouble());
                        snap.tplimit = swapDouble(in.readDouble());
                        snap.equity = swapDouble(in.readDouble());

                        tmp.add(snap);
                    }

                }
                catch (Exc e) {}
                catch (IOException e) {}

                senthist_list = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.dataSentimentHistory();
                    }
                });
            }
        };
        thread.start();
    }

    public void putSent(DataOutputStream out, SentimentSnapshot snap) throws IOException {
        out.writeInt(swap(snap.cur_pres.size()));
        for (Integer i : snap.cur_pres)
            out.writeInt(swap(i));
        out.writeInt(swap(snap.pair_pres.size()));
        for (Integer i : snap.pair_pres)
            out.writeInt(swap(i));
        out.writeInt(swap(snap.comment.length()));
        out.writeBytes(snap.comment);
        out.writeDouble(swapDouble(snap.fmlevel));
        out.writeDouble(swapDouble(snap.tplimit));
    }

    public void startRefreshCheck() {
        Thread thread = new Thread() {
            @Override
            public void run() {
                SentimentSnapshot snap = new SentimentSnapshot();
                snap.cur_pres = new Vector<>();
                snap.pair_pres = new Vector<>();
                snap.comment = "";
                for (Integer i : MainActivity.last.sent_pair_values)
                    snap.pair_pres.add(i);

                List<EventError> tmp = new Vector<>();

                try {
                    ByteArrayOutputStream dout = new ByteArrayOutputStream();
                    DataOutputStream out = new DataOutputStream(dout);

                    out.writeBytes("errorlist,");
                    putSent(out, snap);

                    byte[] data = get(dout.toByteArray());
                    DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));

                    int err_count = swap(in.readInt());
                    for(int i = 0; i < err_count ; i++) {
                        EventError e = new EventError();

                        e.msg = readStringSwap(in);
                        e.level = swap(in.readInt());
                        e.time = new Date(swap(in.readInt()) * 1000L);

                        tmp.add(e);
                    }

                }
                catch (Exc e) {}
                catch (IOException e) {}

                errors = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        CheckActivity.last.data();
                    }
                });
            }
        };
        thread.start();
    }

    public void startSendSnapshot(final SentimentSnapshot snap) {
        Thread thread = new Thread() {
            @Override
            public void run() {
                List<EventError> tmp = new Vector<>();

                try {
                    ByteArrayOutputStream dout = new ByteArrayOutputStream();
                    DataOutputStream out = new DataOutputStream(dout);

                    out.writeBytes("sendsent,");
                    putSent(out, snap);

                    byte[] data = get(dout.toByteArray());
                }
                catch (Exc e) {}
                catch (IOException e) {}

                errors = tmp;

                MainActivity.last.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.last.setView(MainActivity.MainView.SENTIMENTHISTORY);
                    }
                });
            }
        };
        thread.start();
    }




}


class Exc extends Throwable {
    String msg;
    Exc(String s) {msg = s;}
}

class QuotesData {
    String symbol;
    double ask, bid, point;
}

class Order {
    int ticket;
    Date begin, end;
    int type;
    double size;
    String symbol;
    double open, stoploss, takeprofit;
    double close, commission, swap, profit;
}

class CalEvent {
    int id = -1;
    Date timestamp;
    int impact = 0, direction = 0;
    String title, unit, currency, forecast, previous, actual;

    String GetImpactString() {
        switch (impact) {
            case 1:  return "Low";
            case 2:  return "Medium";
            case 3:  return "High";
            default: return "";
        }
    }

}

class Event {
    String msg;
    int level;
    Date received;
}

class SentimentSnapshot {
    List<Integer> cur_pres, pair_pres;
    String comment;
    Date added;
    double fmlevel=0.0, tplimit=0.1;
    double equity=0.0;
}

class EventError {
    String msg;
    int level=0;
    Date time;
}
