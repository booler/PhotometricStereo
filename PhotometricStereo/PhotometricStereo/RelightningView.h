#pragma once

#include "PixelInfoSetView.h"

class RelightningView : public PixelInfoSetView
{
public:
    RelightningView(PixelInfoSetViewDialog *parent);
    
    ~RelightningView() { Clear(); }

protected:
    // connections
    virtual void ApplySetting(PixelInfoSetView *view) override;

    virtual char *GetMimeType() const override
    {
        return "application/{B4A9B742-3763-40F0-A25E-EE4E72DF91D4}";
    }
protected:
    virtual void initializeGL() override
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        
        GLfloat specular [] = {1.0, 1.0, 1.0, 1.0};
        GLfloat shininess [] = {50.0};
        GLfloat LightPosition_ [] = {1.0, 1.0, 1.0, 0.0};
        GLfloat white_light [] = {1.0, 1.0, 1.0, 1.0};
        GLfloat lmodel_ambient[] = {0.1, 0.1, 0.1, 1.0};
        glShadeModel(GL_SMOOTH);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, LightPosition_);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
        glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
    }

    virtual void resizeGL(int w, int h) override
    {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        double view_cube[6];
        if (static_cast<double>(w) / h >= static_cast<double>(Region_.width()) / Region_.height())
        {
            view_cube[0] = (Region_.width() - Region_.height() * static_cast<double>(w) / h) / 2.0;
            view_cube[1] = (Region_.width() + Region_.height() * static_cast<double>(w) / h) / 2.0;
            view_cube[2] = 0; 
            view_cube[3] = Region_.height();  
            view_cube[4] = -100.0 * Region_.width(); 
            view_cube[5] = 100.0 * Region_.width();
        }
        else
        {
            view_cube[0] = 0;
            view_cube[1] = Region_.width();
            view_cube[2] = (Region_.height() - Region_.width() * static_cast<double>(h) / w) / 2.0;
            view_cube[3] = (Region_.height() + Region_.width() * static_cast<double>(h) / w) / 2.0;
            view_cube[4] = -100.0 * Region_.height();
            view_cube[5] = 100.0 * Region_.height();
        }

        glOrtho(view_cube[0], view_cube[1], view_cube[2], view_cube[3], view_cube[4], view_cube[5]);

      }

    virtual void mouseMoveEvent(QMouseEvent *event)
    {
        if (RenderMode_ == RelightningMode)
        {
            int h = this->height();
            int w = this->width();
            int x = event->pos().x();
            int y = event->pos().y();
            double dy = h / 2.0 - y;
            double dx = x - w / 2.0;
            double a = w / 2.0;
            double b = h / 2.0;
            double c = (a + b) / 2.0;
            double t = max((1 - dx * dx / a / a - dy * dy / b / b) * c * c, 0.0);
            double dz = sqrt(t);
            uvec3 ul(dx, dy, dz);
            LightPosition_[0] = ul.X;
            LightPosition_[1] = ul.Y;
            LightPosition_[2] = ul.Z;
            LightPosition_[3] = 0.0;
        }
        Update();

    }

    virtual void keyReleaseEvent(QKeyEvent *evt) override
    {
        __super::keyReleaseEvent(evt);
        if (evt->isAccepted())
            return;
        if (evt->key() == Qt::Key_Space)
        {
            RenderMode_ = RenderMode((RenderMode_ + 1) % RenderModeCount);
            Update();
        }
    }

    
    virtual void render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (RenderMode_ == RelightningMode)
        {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
        }
        else
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_LIGHT0);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLightfv(GL_LIGHT0, GL_POSITION, LightPosition_);
        glLoadIdentity();
        glTranslated(Region_.width() / 2.0, Region_.height() / 2.0, 0.0);
        //glScaled(0.8, 0.8, 0.8);
        glTranslated(-Region_.width() / 2.0, Region_.height() / 2.0, 0.0);
        glScaled(1.0, -1.0, 1.0);

        int h = Region_.height();
        int w = Region_.width();

        for (int i = 0; i < h - 1; i++)
        {
            for (int j = 0; j < w - 1; j++)
            {
                PixelInfo &ij1 = Pixels_[i * w + j];
                PixelInfo &ij2 = Pixels_[(i + 1) * w + j];
                PixelInfo &ij3 = Pixels_[(i + 1) * w + j + 1];
                PixelInfo &ij4 = Pixels_[i * w + j + 1];
                if (SkipDarkPixel_ && (ij1.DarkPixel() && ij2.DarkPixel() && ij3.DarkPixel() && ij4.DarkPixel()))
                    continue;
                glBegin(GL_QUADS);
                glColor3d((ij1.Normal.X + 1) / 2.0, (ij1.Normal.Y + 1) / 2.0, (ij1.Normal.Z));
                glNormal3d(ij1.Normal.X, ij1.Normal.Y, ij1.Normal.Z);
                glVertex3d(ij1.Position.X, 
                    ij1.Position.Y, 
                    0.0);

                glColor3d((ij2.Normal.X + 1) / 2.0, (ij2.Normal.Y + 1) / 2.0, (ij2.Normal.Z));
                glNormal3d(ij2.Normal.X, ij2.Normal.Y, ij2.Normal.Z);
                glVertex3d(ij2.Position.X, 
                    ij2.Position.Y, 
                    0.0);

                glColor3d((ij3.Normal.X + 1) / 2.0, (ij3.Normal.Y + 1) / 2.0, (ij3.Normal.Z));
                glNormal3d(ij3.Normal.X, ij3.Normal.Y, ij3.Normal.Z);
                glVertex3d(ij3.Position.X, 
                    ij3.Position.Y, 
                    0.0);

                glColor3d((ij4.Normal.X + 1) / 2.0, (ij4.Normal.Y + 1) / 2.0, (ij4.Normal.Z));
                glNormal3d(ij4.Normal.X, ij4.Normal.Y, ij4.Normal.Z);
                glVertex3d(ij4.Position.X, 
                    ij4.Position.Y, 
                    0.0);
                glEnd();
            }
        }

        glEnd();
    }
private:  
    enum RenderMode { RelightningMode, NormalMode, /* count */ RenderModeCount };
    RenderMode RenderMode_;

    GLfloat LightPosition_[4];

};

