package fi.zzz.overlook;

import android.content.Context;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;

import java.util.List;

public class CandlestickView extends View {
    private Paint mTextPaint;
    private Paint mPiePaint;
    private Paint mShadowPaint;
    private Paint upcolor, downcolor;
    float mTextHeight = 0;
    int mTextColor = 0xff000000;
    int w = 0, h = 0;
    int count = 0;
    int border = 5;
    int densityDpi = 96;
    int div = 8;
    int shift = 0;
    int pt = 0;
    float prev_x = 0;
    float x_sum = 0;

    public CandlestickView(Context context, AttributeSet attrs) {
        super(context, attrs);

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();
        switch (event.getAction()) {
            case MotionEvent.ACTION_MOVE:
                if (prev_x != 0) {
                    x_sum += (x - prev_x);
                    int shift_diff = (int)(x_sum / div);
                    x_sum -= shift_diff * div;
                    shift += shift_diff;
                }
                invalidate();
                break;
            default:
                break;
        }
        prev_x = x;

        // Invalidate the whole view. If the view is visible.
        invalidate();
        return true;
    }

    private void init() {
        mTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mTextPaint.setColor(mTextColor);
        if (mTextHeight == 0) {
            mTextHeight = mTextPaint.getTextSize();
        } else {
            mTextPaint.setTextSize(mTextHeight);
        }

        mPiePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPiePaint.setStyle(Paint.Style.FILL);
        mPiePaint.setTextSize(mTextHeight);

        mShadowPaint = new Paint(0);
        mShadowPaint.setColor(0xff101010);
        mShadowPaint.setMaskFilter(new BlurMaskFilter(8, BlurMaskFilter.Blur.NORMAL));


        upcolor = new Paint(Paint.ANTI_ALIAS_FLAG);
        upcolor.setColor(0xff38d496); // 0xff008a63
        upcolor.setStyle(Paint.Style.FILL);

        downcolor = new Paint(Paint.ANTI_ALIAS_FLAG);
        downcolor.setColor(0xff1c5596); // 0xff173a63
        downcolor.setStyle(Paint.Style.FILL);

    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // Try for a width based on our minimum
        int minw = getPaddingLeft() + getPaddingRight() + getSuggestedMinimumWidth();
        int w = resolveSizeAndState(minw, widthMeasureSpec, 1);

        // Whatever the width ends up being, ask for a height that would let the pie
        // get as big as it can
        int minh = MeasureSpec.getSize(w) - getPaddingBottom() + getPaddingTop();
        int h = resolveSizeAndState(MeasureSpec.getSize(w), heightMeasureSpec, 0);

        setMeasuredDimension(w, h);
        this.w = w;
        this.h = h;

        DisplayMetrics metrics = getResources().getDisplayMetrics();
        densityDpi = (int)(metrics.density * 160f);
        border = densityDpi / 96 * 5;
        div = densityDpi / 96 * 8;
        pt = densityDpi / 96;
    }

    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        if (mTextPaint == null)
            init();




        int f, pos, x, y, c;
        double diff;
        List<Double> opens = AppService.last.opens;
        List<Double> lows  = AppService.last.lows;
        List<Double> highs = AppService.last.highs;

        count = w / div;
        c = opens.size();
        if (shift > c) shift = c;
        if (shift < 0) shift = 0;
        int begin = c - (count + shift);
        int end = c - (shift);
        if (begin < 0) begin = 0;
        if (end < 0) end = 0;
        if (begin > c) begin = c;
        if (end > c) end = c;

        double hi = -Double.MAX_VALUE, lo = +Double.MAX_VALUE;
        for(int i = begin; i < end; i++)
            if (lows.get(i) < lo) lo = lows.get(i);
        for(int i = begin; i < end; i++)
            if (highs.get(i) > hi) hi = highs.get(i);



        f = 2;
        x = border;
        y = 0;
        diff = hi - lo;

        Path polygon = new Path();
        Path line = new Path();


        for(int i = 0; i < count; i++) {
            double O, H, L, C;
            pos = c - (count + shift - i);
            if (pos >= c || pos < 0) continue;

            double open  = opens.get(pos);
            double low   = lows.get(pos);
            double high  = highs.get(pos);
            double close =
                    pos+1 < c ?
                            opens.get(pos+1) :
                            open;

            O = (1 - (open  - lo) / diff) * h;
            H = (1 - (high  - lo) / diff) * h;
            L = (1 - (low   - lo) / diff) * h;
            C = (1 - (close - lo) / diff) * h;

            polygon.reset();
            polygon.moveTo((int)(x+i*div+f),		(int)(y+O));
            polygon.lineTo((int)(x+(i+1)*div-f),	(int)(y+O));
            polygon.lineTo((int)(x+(i+1)*div-f),	(int)(y+C));
            polygon.lineTo((int)(x+i*div+f),		(int)(y+C));
            polygon.lineTo((int)(x+i*div+f),		(int)(y+O));

            line.reset();
            line.moveTo((int)(x+(i+0.5)*div), (int)(y+H));
            line.lineTo((int)(x+(i+0.5)*div), (int)(y+L));
            line.lineTo((int)(x+(i+0.5)*div+pt), (int)(y+L));
            line.lineTo((int)(x+(i+0.5)*div+pt), (int)(y+H));


            {
                if (C < O) {
                    canvas.drawPath(line, upcolor);
                    canvas.drawPath(polygon, upcolor);
                } else {
                    canvas.drawPath(line, downcolor);
                    canvas.drawPath(polygon, downcolor);
                }
            }
        }

        // Draw the shadow
        /*canvas.drawOval(
                mShadowBounds,
                mShadowPaint
        );

        // Draw the label text

        // Draw the pie slices
        for (int i = 0; i < mData.size(); ++i) {
            Item it = mData.get(i);
            mPiePaint.setShader(it.mShader);
            canvas.drawArc(mBounds,
                    360 - it.mEndAngle,
                    it.mEndAngle - it.mStartAngle,
                    true, mPiePaint);
        }

        // Draw the pointer
        canvas.drawLine(mTextX, mPointerY, mPointerX, mPointerY, mTextPaint);
        canvas.drawCircle(mPointerX, mPointerY, mPointerSize, mTextPaint);*/
    }
}

