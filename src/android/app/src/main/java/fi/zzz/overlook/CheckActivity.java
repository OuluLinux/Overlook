package fi.zzz.overlook;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.List;
import java.util.Vector;

public class CheckActivity extends Activity {
    public static CheckActivity last;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        last = this;

        setContentView(R.layout.check);

        AppService.last.startRefreshCheck();
    }

    void data() {
        List<String> itemname = new Vector<>();
        for (EventError e : AppService.last.errors)
            itemname.add(e.msg);

        EventErrorAdapter adapter = new EventErrorAdapter(this, itemname, AppService.last.errors);
        ListView list = (ListView)findViewById(R.id.errors);
        list.setAdapter(adapter);
    }

}

class EventErrorAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<EventError> values;

    public EventErrorAdapter (Activity context, List<String> items, List<EventError> values) {
        super(context, R.layout.errorlist, items);

        this.context=context;
        this.values=values;
    }

    public View getView(int position, View view, ViewGroup parent) {
        LayoutInflater inflater = context.getLayoutInflater();
        View rowView = inflater.inflate(R.layout.errorlist, null,true);

        EventError e = values.get(position);

        TextView error = (TextView) rowView.findViewById(R.id.error);
        error.setText(e.msg);

        if (e.level == 2)
            error.setTextColor(Color.rgb(111,0,0));
        else if (e.level == 1)
            error.setTextColor(Color.rgb(111,111,0));

        return rowView;

    };
}