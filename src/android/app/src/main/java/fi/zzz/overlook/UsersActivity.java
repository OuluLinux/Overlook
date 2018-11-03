/*
package fi.zzz.overlook;

import android.app.Activity;
import android.app.ListActivity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class UsersActivity extends Activity {
    static UsersActivity last;

    ListView list;

    public UsersActivity() {
        last = this;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_userlist);

        refresh();
    }

    void refresh() {
        final List<String> itemname = new ArrayList<String>();
        final List<Integer> user_ids = new ArrayList<Integer>();

        Channel ch = AppService.last.getActiveChannel();
        for (Integer user_id : ch.userlist) {
            User u = AppService.last.users.get(user_id);
            if (u != null) {
                itemname.add(u.name);
                user_ids.add(user_id);
            }
        }

        CustomListAdapter adapter = new CustomListAdapter(this, itemname, user_ids);
        list = (ListView)findViewById(R.id.list);
        list.setAdapter(adapter);

        list.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                                    int position, long id) {
                // TODO Auto-generated method stub
                String selected_item = itemname.get(position);
                Toast.makeText(getApplicationContext(), selected_item, Toast.LENGTH_SHORT).show();

            }
        });
    }

}

class CustomListAdapter extends ArrayAdapter<String> {

    private final Activity context;
    private final List<String> itemname;
    private final List<Integer> userid;

    public CustomListAdapter(Activity context, List<String> itemname, List<Integer> userid) {
        super(context, R.layout.userlist, itemname);
        // TODO Auto-generated constructor stub

        this.context=context;
        this.itemname=itemname;
        this.userid=userid;
    }

    public View getView(int position,View view,ViewGroup parent) {
        LayoutInflater inflater=context.getLayoutInflater();
        View rowView=inflater.inflate(R.layout.userlist, null,true);

        TextView txtTitle = (TextView) rowView.findViewById(R.id.item);
        ImageView imageView = (ImageView) rowView.findViewById(R.id.icon);
        TextView extratxt = (TextView) rowView.findViewById(R.id.textView1);

        User u = AppService.last.users.get(userid.get(position));
        txtTitle.setText(itemname.get(position));
        if (u != null) {
            imageView.setImageBitmap(u.profile_image);

            String xtra = Integer.toString(u.age) + " years old " + (u.gender ? "male" : "female");
            extratxt.setText(xtra);
        }
        return rowView;

    };
}
*/