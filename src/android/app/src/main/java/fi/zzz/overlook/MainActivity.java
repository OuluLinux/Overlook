package fi.zzz.overlook;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.os.Bundle;

import com.google.android.gms.maps.model.Marker;

import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class MainActivity extends AppCompatActivity {

    public static MainActivity last;

    private DrawerLayout mDrawerLayout;
    private HashMap<Integer, Marker> markers;
    private Marker my_marker;

    public Lock lock = new ReentrantLock();
    public int graph_prev_tf = 4, graph_prev_sym = 0;
    public List<Integer> sent_cur_values, sent_pair_values;

    private static final String TAG = "Overlook";
    private static final int REQUEST_PERMISSION_LOCATION = 255; // int should be between 0 and 255

    enum MainView {QUOTES, CANDLESTICKS, ORDERS, HISTORY, CALENDAR, EVENTS, SENTIMENTHISTORY, SENTIMENT};
    private MainView last_view;


    public MainActivity() {
        last = this;
        markers = new HashMap<>();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        last = this;

        try {
            super.onCreate(savedInstanceState);
            setView(MainView.QUOTES);


            Intent intent = getIntent();
            String action = intent.getAction();

            if (!action.isEmpty()) {
                if (action == Intent.ACTION_MAIN) {

                }
            }

            // Start background service
            if (AppService.last == null) {
                Intent i = new Intent(this, AppService.class);
                i.putExtra("name", "Overlook service");
                startService(i);
            }
            else {
                postRefreshGui();
            }

        } catch (java.lang.NullPointerException e) {
            Log.e(TAG, "System error");
            System.exit(1);
        }

    }


    void startOnboarding() {
        startActivity(new Intent(this, OnboardingActivity.class));
    }

    void startSettings() {
        startActivity(new Intent(this, SettingsActivity.class));
    }






    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                mDrawerLayout.openDrawer(GravityCompat.START);
                return true;
        }
        return super.onOptionsItemSelected(item);
    }


    void postSetTitle(final String s) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toolbar toolbar = findViewById(R.id.toolbar);
                if (toolbar != null)
                    toolbar.setTitle(s);
            }
        });
    }

    public void postStartOnboarding() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                startOnboarding();
            }
        });
    }

    public void postRefreshGui() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                refreshGui();
            }
        });
    }



    static public Bitmap getCroppedBitmap(Bitmap bitmap) {
        Bitmap output = Bitmap.createBitmap(bitmap.getWidth(),
                bitmap.getHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(output);

        final int color = 0xff424242;
        final Paint paint = new Paint();
        final Rect rect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());

        paint.setAntiAlias(true);
        canvas.drawARGB(0, 0, 0, 0);
        paint.setColor(color);
        canvas.drawCircle(bitmap.getWidth() / 2, bitmap.getHeight() / 2,
                bitmap.getWidth() / 2, paint);
        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
        canvas.drawBitmap(bitmap, rect, rect, paint);
        return output;
    }


    void setView(MainView v) {
        last_view = v;

        String title = "Overlook";
        if (last_view == MainView.QUOTES) {
            setContentView(R.layout.quotes);
            title = getApplicationContext().getResources().getString(R.string.quotes);
        }
        else if (last_view == MainView.CANDLESTICKS) {
            setContentView(R.layout.graph);
            title = getApplicationContext().getResources().getString(R.string.charts);
        }
        else if (last_view == MainView.ORDERS) {
            setContentView(R.layout.orders);
            title = getApplicationContext().getResources().getString(R.string.orders);
        }
        else if (last_view == MainView.HISTORY) {
            setContentView(R.layout.orders);
            title = getApplicationContext().getResources().getString(R.string.history);
        }
        else if (last_view == MainView.CALENDAR) {
            setContentView(R.layout.calendar);
            title = getApplicationContext().getResources().getString(R.string.calendar);
        }
        else if (last_view == MainView.EVENTS) {
            setContentView(R.layout.events);
            title = getApplicationContext().getResources().getString(R.string.events);
        }
        else if (last_view == MainView.SENTIMENTHISTORY) {
            setContentView(R.layout.senthistory);
            title = getApplicationContext().getResources().getString(R.string.sentimenthistory);
        }
        else if (last_view == MainView.SENTIMENT) {
            setContentView(R.layout.sent);
            title = getApplicationContext().getResources().getString(R.string.sentiment);
        }



        // Navigation bar
        mDrawerLayout = findViewById(R.id.drawer_layout);
        NavigationView navigationView = findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(
                new NavigationView.OnNavigationItemSelectedListener() {
                    @Override
                    public boolean onNavigationItemSelected(@NonNull MenuItem menuItem) {
                        // set item as selected to persist highlight
                        //menuItem.setChecked(true);

                        // close drawer when item is tapped
                        mDrawerLayout.closeDrawers();

                        int id = menuItem.getItemId();
                        if (id == R.id.nav_settings)
                            startSettings();
                        if (id == R.id.nav_quotes)
                            setView(MainView.QUOTES);
                        if (id == R.id.nav_charts)
                            setView(MainView.CANDLESTICKS);
                        if (id == R.id.nav_orders)
                            setView(MainView.ORDERS);
                        if (id == R.id.nav_history)
                            setView(MainView.HISTORY);
                        if (id == R.id.nav_calendar)
                            setView(MainView.CALENDAR);
                        if (id == R.id.nav_events)
                            setView(MainView.EVENTS);
                        if (id == R.id.nav_sentimenthistory)
                            setView(MainView.SENTIMENTHISTORY);
                        if (id == R.id.nav_sentiment)
                            setView(MainView.SENTIMENT);

                        // Add code here to update the UI based on the item selected
                        // For example, swap UI fragments here
                        return true;
                    }
                });


        // Set the toolbar as the action bar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        toolbar.setTitle(title);
        ActionBar actionbar = getSupportActionBar();
        actionbar.setDisplayHomeAsUpEnabled(true);
        actionbar.setHomeAsUpIndicator(R.drawable.ic_menu);



        refreshGui();
    }

    void refreshGui() {
        if (AppService.last == null) return;

        AppService.last.lock.lock();

        if (last_view == MainView.QUOTES) {
            refreshQuotes();
        }
        else if (last_view == MainView.CANDLESTICKS) {
            refreshGraph();
        }
        else if (last_view == MainView.ORDERS) {
            refreshOrders();
        }
        else if (last_view == MainView.HISTORY) {
            refreshHistory();
        }
        else if (last_view == MainView.CALENDAR) {
            refreshCalendar();
        }
        else if (last_view == MainView.EVENTS) {
            dataEvents();
        }
        else if (last_view == MainView.SENTIMENTHISTORY) {
            refreshSentimentHistory();
        }
        else if (last_view == MainView.SENTIMENT) {
            dataSentiment();
        }

        AppService.last.lock.unlock();
    }

    void refreshQuotes() {
        AppService.last.startRefreshQuotes();
    }

    void refreshGraph() {
        AppService.last.startRefreshGraph(graph_prev_sym, graph_prev_tf);
    }

    void refreshOrders() {
        AppService.last.startRefreshOrders();
    }

    void refreshHistory() {
        AppService.last.startRefreshHistory();
    }

    void refreshCalendar() {
        AppService.last.startRefreshCalendar();
    }

    void refreshSentimentHistory() { AppService.last.startRefreshSentimentHistory(); }

    void dataQuotes() {
        List<String> itemname = new Vector<>();
        for (QuotesData q : AppService.last.quote_values)
            itemname.add(q.symbol);

        QuotesAdapter adapter = new QuotesAdapter(this, itemname, AppService.last.quote_values);
        ListView list = (ListView)findViewById(R.id.quotes_list);
        list.setAdapter(adapter);

        list.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                                    int position, long id) {
                // TODO Auto-generated method stub
                QuotesData d = AppService.last.quote_values.get(position);

                //String selected_item = itemname.get(position);
                //Toast.makeText(getApplicationContext(), selected_item, Toast.LENGTH_SHORT).show();

            }
        });
    }

    public class GraphSymbolSelector implements AdapterView.OnItemSelectedListener {
        public void onNothingSelected(AdapterView<?> parent) {}
        public void onItemSelected(AdapterView<?> parent, View v, int position, long id) {
            if (MainActivity.last.graph_prev_sym != position) {
                MainActivity.last.graph_prev_sym = position;
                MainActivity.last.refreshGraph();
            }
        }
    }

    public class GraphTfSelector implements AdapterView.OnItemSelectedListener {
        public void onNothingSelected(AdapterView<?> parent) {}
        public void onItemSelected(AdapterView<?> parent, View v, int position, long id) {
            if (MainActivity.last.graph_prev_tf != position) {
                MainActivity.last.graph_prev_tf = position;
                MainActivity.last.refreshGraph();
            }
        }
    }

    void dataGraph() {
        CandlestickView v = findViewById(R.id.candlesticks);
        v.invalidate();

        Spinner graphsymbol = findViewById(R.id.graphsymbol);
        ArrayAdapter<String> graphsymbol_adapter =
                new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, AppService.last.symbols);
        graphsymbol.setAdapter(graphsymbol_adapter);
        graphsymbol.setOnItemSelectedListener(new GraphSymbolSelector());
        graphsymbol.setSelection(graph_prev_sym);

        Spinner graphtf = findViewById(R.id.graphtf);
        ArrayAdapter<String> graphtf_adapter =
                new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, AppService.last.tflist);
        graphtf.setAdapter(graphtf_adapter);
        graphtf.setOnItemSelectedListener(new GraphTfSelector());
        graphtf.setSelection(graph_prev_tf);
    }

    public void dataOrders() {
        List<String> itemname = new Vector<>();
        for (Order o : AppService.last.open_orders)
            itemname.add(o.symbol);

        OrdersAdapter adapter = new OrdersAdapter(this, itemname, AppService.last.open_orders);
        ListView list = (ListView)findViewById(R.id.orders_list);
        list.setAdapter(adapter);

        TextView account = findViewById(R.id.account);
        account.setText("Balance: " + Double.toString(AppService.last.balance) + "\nEquity: " +
            Double.toString(AppService.last.equity) + "\nFree-margin: " +
            Double.toString(AppService.last.freemargin));
    }

    public void dataHistoryOrders() {
        List<String> itemname = new Vector<>();
        for (Order o : AppService.last.history_orders)
            itemname.add(o.symbol);

        OrdersAdapter adapter = new OrdersAdapter(this, itemname, AppService.last.history_orders);
        ListView list = (ListView)findViewById(R.id.orders_list);
        list.setAdapter(adapter);

        TextView account = findViewById(R.id.account);
        account.setText("Balance: " + Double.toString(AppService.last.balance) + "\nEquity: " +
                Double.toString(AppService.last.equity) + "\nFree-margin: " +
                Double.toString(AppService.last.freemargin));
    }

    public void dataCalendar() {
        List<String> itemname = new Vector<>();
        for (CalEvent o : AppService.last.cal_events)
            itemname.add(o.title);

        CalendarAdapter adapter = new CalendarAdapter(this, itemname, AppService.last.cal_events);
        ListView list = (ListView)findViewById(R.id.calendar_list);
        list.setAdapter(adapter);
    }

    public void dataEvents() {
        List<String> itemname = new Vector<>();
        for (Event e : AppService.last.events)
            itemname.add(e.msg);

        EventAdapter adapter = new EventAdapter(this, itemname, AppService.last.events);
        ListView list = (ListView)findViewById(R.id.events_list);
        list.setAdapter(adapter);
    }

    public void dataSentimentHistory() {
        List<String> itemname = new Vector<>();
        for (SentimentSnapshot snap : AppService.last.senthist_list)
            itemname.add(snap.comment);

        SentimentAdapter adapter = new SentimentAdapter(this, itemname, AppService.last.senthist_list);
        ListView list = (ListView)findViewById(R.id.sentimenthistory_list);
        list.setAdapter(adapter);

    }

    public void initSentiment() {
        sent_cur_values = new Vector<>();

        List<String> currencies = new Vector<>();
        for (String s: AppService.last.sent_currencies)
            sent_cur_values.add(0);

        sent_pair_values = new Vector<>();

        List<String> pairs = new Vector<>();
        for (String s: AppService.last.sent_pairs)
            sent_pair_values.add(0);
    }

    public void dataSentiment() {
        List<String> currencies = new Vector<>();
        for (String s: AppService.last.sent_currencies)
            currencies.add(s);

        SentimentCurrencyAdapter currency_adapter = new SentimentCurrencyAdapter (this, currencies, AppService.last.sent_currencies);
        ListView curlist = (ListView)findViewById(R.id.cur_list);
        curlist.setAdapter(currency_adapter );

        List<String> pairs = new Vector<>();
        for (String s: AppService.last.sent_pairs)
            pairs.add(s);

        SentimentPairAdapter pair_adapter = new SentimentPairAdapter(this, pairs, AppService.last.sent_pairs);
        ListView pairlist = (ListView)findViewById(R.id.pair_list);
        pairlist.setAdapter(pair_adapter );

        Button check = findViewById(R.id.check);
        check.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.last, CheckActivity.class));
            }
        });

        final EditText comment = findViewById(R.id.comment);

        Button send = findViewById(R.id.send);
        send.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                SentimentSnapshot snap = new SentimentSnapshot();
                snap.cur_pres = new Vector<>();
                snap.pair_pres = new Vector<>();
                snap.comment = comment.getText().toString();
                for (Integer i : MainActivity.last.sent_cur_values)
                    snap.cur_pres.add(i);
                for (Integer i : MainActivity.last.sent_pair_values)
                    snap.pair_pres.add(i);

                AppService.last.startSendSnapshot(snap);
            }
        });
    }

    public void setCurPairPressures() {
        Map<String, Integer> pres = new HashMap<>();

        for (int i = 0; i < sent_cur_values.size(); i++) {
            String cur = AppService.last.sent_currencies.get(i);
            int value = sent_cur_values.get(i);
            pres.put(cur, value);
        }

        for (int i = 0; i < sent_pair_values.size(); i++) {
            String pair = AppService.last.sent_pairs.get(i);
            String a = pair.substring(0, 3);
            String b = pair.substring(3, 6);
            int ap = 0;
            if (pres.containsKey(a)) ap = pres.get(a);
            int bp = 0;
            if (pres.containsKey(b)) bp = pres.get(b);
            int p = ap - bp;
            sent_pair_values.set(i, p);
        }

        dataSentiment();
    }
}










class QuotesAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<QuotesData> quotes_values;

    public QuotesAdapter(Activity context, List<String> items, List<QuotesData> quotes_values) {
        super(context, R.layout.quoteslist, items);

        this.context=context;
        this.quotes_values=quotes_values;
    }

    public View getView(int position, View view, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.quoteslist, null,true);

        TextView symbol = (TextView) rowView.findViewById(R.id.symbol);
        TextView spread = (TextView) rowView.findViewById(R.id.spread);
        TextView bid = (TextView) rowView.findViewById(R.id.bid);
        TextView ask = (TextView) rowView.findViewById(R.id.profit);

        QuotesData q = quotes_values.get(position);

        symbol.setText(q.symbol);
        spread.setText("Spread: " + Integer.toString((int)((q.ask - q.bid) / q.point)));
        bid.setText(Double.toString(q.bid));
        ask.setText(Double.toString(q.ask));

        return rowView;

    };
}

class OrdersAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<Order> values;

    public OrdersAdapter (Activity context, List<String> items, List<Order> values) {
        super(context, R.layout.quoteslist, items);

        this.context=context;
        this.values=values;
    }

    public View getView(int position, View view, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.orderslist, null,true);

        TextView symbol = (TextView) rowView.findViewById(R.id.symbol);
        TextView begin = (TextView) rowView.findViewById(R.id.begin);
        TextView end = (TextView) rowView.findViewById(R.id.end);
        TextView lots = (TextView) rowView.findViewById(R.id.lots);
        TextView profit = (TextView) rowView.findViewById(R.id.profit);

        Order o = values.get(values.size() - 1 - position);

        symbol.setText(o.symbol);
        begin.setText(o.begin.toString());
        if (o.end != null)
            end.setText(o.end.toString());
        else
            end.setText("");
        lots.setText(Double.toString(o.size));
        profit.setText(Double.toString(o.profit));

        return rowView;

    };
}


class CalendarAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<CalEvent> values;
    Date currentTime;

    public CalendarAdapter (Activity context, List<String> items, List<CalEvent> values) {
        super(context, R.layout.quoteslist, items);

        this.context=context;
        this.values=values;

        currentTime = Calendar.getInstance().getTime();
    }

    public View getView(int position, View view, ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.calendarlist, null,true);

        TextView title = (TextView) rowView.findViewById(R.id.title);
        TextView currency = (TextView) rowView.findViewById(R.id.currency);
        TextView time = (TextView) rowView.findViewById(R.id.time);
        TextView actual = (TextView) rowView.findViewById(R.id.actual);
        TextView forecast = (TextView) rowView.findViewById(R.id.forecast);
        TextView previous = (TextView) rowView.findViewById(R.id.previous);

        CalEvent e = values.get(position);

        title.setText(e.title);
        currency.setText(e.currency);
        time.setText((e.timestamp.compareTo(currentTime) > 0 ? "UPCOMING " : "") + e.timestamp.toString());
        actual.setText(e.actual);
        forecast.setText(e.forecast);
        previous.setText(e.previous);

        return rowView;

    };
}


class EventAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<Event> values;

    public EventAdapter(Activity context, List<String> items, List<Event> values) {
        super(context, R.layout.quoteslist, items);

        this.context=context;
        this.values=values;
    }

    public View getView(int position, View view, ViewGroup parent) {
        LayoutInflater inflater = context.getLayoutInflater();
        View rowView = inflater.inflate(R.layout.eventslist, null,true);

        TextView event = (TextView) rowView.findViewById(R.id.event);
        TextView received = (TextView) rowView.findViewById(R.id.received);

        Event e = values.get(position);

        event.setText(e.msg);
        received.setText(e.received.toString());

        if (e.level == 2)
            event.setTextColor(Color.rgb(111,0,0));
        else if (e.level == 1)
            event.setTextColor(Color.rgb(111,111,0));

        return rowView;

    };
}


class SentimentAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<SentimentSnapshot> values;

    public SentimentAdapter(Activity context, List<String> items, List<SentimentSnapshot> values) {
        super(context, R.layout.quoteslist, items);

        this.context=context;
        this.values=values;
    }

    public View getView(int position, View view, ViewGroup parent) {
        LayoutInflater inflater = context.getLayoutInflater();
        View rowView = inflater.inflate(R.layout.senthistorylist, null,true);

        TextView comment = (TextView) rowView.findViewById(R.id.comment);
        TextView time = (TextView) rowView.findViewById(R.id.time);
        TextView profit = (TextView) rowView.findViewById(R.id.profit);

        SentimentSnapshot snap = values.get(values.size() - 1 - position);

        comment.setText(snap.comment);
        time.setText(snap.added.toString());
        if (position > 0) {
            SentimentSnapshot prev = values.get(values.size() - position);
            profit.setText(Double.toString(snap.equity - prev.equity));
        }

        return rowView;

    };
}


class SentimentCurrencyAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<String> values;

    public SentimentCurrencyAdapter(Activity context, List<String> items, List<String> values) {
        super(context, R.layout.sentlist, items);

        this.context=context;
        this.values=values;
    }

    public View getView(final int position, View view, ViewGroup parent) {
        LayoutInflater inflater = context.getLayoutInflater();
        View rowView = inflater.inflate(R.layout.sentlist, null,true);

        TextView symbol = (TextView) rowView.findViewById(R.id.symbol);
        symbol.setText(values.get(position));

        StartPointSeekBar bar = rowView.findViewById(R.id.slider);
        bar.setProgress(MainActivity.last.sent_cur_values.get(position));
        bar.setOnSeekBarChangeListener(new StartPointSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onOnSeekBarValueChange(StartPointSeekBar bar, double value) {
                int cur = (int)value;
                int prev = MainActivity.last.sent_cur_values.get(position);
                if (cur != prev) {
                    MainActivity.last.sent_cur_values.set(position, (int) value);
                    MainActivity.last.setCurPairPressures();
                }
            }
        });

        return rowView;

    };
}

class SentimentPairAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<String> values;

    public SentimentPairAdapter (Activity context, List<String> items, List<String> values) {
        super(context, R.layout.sentlist, items);

        this.context=context;
        this.values=values;
    }

    public View getView(final int position, View view, ViewGroup parent) {
        LayoutInflater inflater = context.getLayoutInflater();
        View rowView = inflater.inflate(R.layout.sentlist, null,true);

        TextView symbol = (TextView) rowView.findViewById(R.id.symbol);
        symbol.setText(values.get(position));

        StartPointSeekBar bar = rowView.findViewById(R.id.slider);
        bar.setProgress(MainActivity.last.sent_pair_values.get(position));
        bar.setOnSeekBarChangeListener(new StartPointSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onOnSeekBarValueChange(StartPointSeekBar bar, double value) {
                MainActivity.last.sent_pair_values.set(position, (int) value);
            }
        });

        return rowView;

    };
}


