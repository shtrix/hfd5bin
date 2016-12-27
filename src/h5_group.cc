#include <node.h>
#include <string>
#include <cstring>
//#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <functional>

#include "file.h"
#include "group.h"
#include "int64.hpp"
#include "filters.hpp"
#include "H5LTpublic.h"
#include "H5PTpublic.h"
#include "H5Lpublic.h"

namespace NodeHDF5 {

    using namespace v8;

    Persistent<FunctionTemplate> Filters::Constructor;

    Group::Group(hid_t id) : Methods(id) {
    }

    Persistent<FunctionTemplate> Group::Constructor;

    void Group::Initialize () {

        // instantiate constructor template
        Local<FunctionTemplate> t = FunctionTemplate::New(v8::Isolate::GetCurrent(), New);

        // set properties
        t->SetClassName(String::NewFromUtf8(v8::Isolate::GetCurrent(), "Group"));
        t->InstanceTemplate()->SetInternalFieldCount(1);
        // member method prototypes
        NODE_SET_PROTOTYPE_METHOD(t, "create", Create);
        NODE_SET_PROTOTYPE_METHOD(t, "open", Open);
        NODE_SET_PROTOTYPE_METHOD(t, "openGroup", OpenGroup);
        NODE_SET_PROTOTYPE_METHOD(t, "refresh", Refresh);
        NODE_SET_PROTOTYPE_METHOD(t, "flush", Flush);
        NODE_SET_PROTOTYPE_METHOD(t, "copy", Copy);
        NODE_SET_PROTOTYPE_METHOD(t, "move", Move);
        NODE_SET_PROTOTYPE_METHOD(t, "link", Link);
        NODE_SET_PROTOTYPE_METHOD(t, "delete", Delete);
        NODE_SET_PROTOTYPE_METHOD(t, "close", Close);
        NODE_SET_PROTOTYPE_METHOD(t, "getNumAttrs", GetNumAttrs);
        NODE_SET_PROTOTYPE_METHOD(t, "getNumObjs", GetNumObjs);
        NODE_SET_PROTOTYPE_METHOD(t, "getMemberNames", GetMemberNames);
        NODE_SET_PROTOTYPE_METHOD(t, "getMemberNamesByCreationOrder", GetMemberNamesByCreationOrder);
        NODE_SET_PROTOTYPE_METHOD(t, "getChildType", GetChildType);
        NODE_SET_PROTOTYPE_METHOD(t, "getDatasetType", getDatasetType);
        NODE_SET_PROTOTYPE_METHOD(t, "getDataType", getDataType);
        NODE_SET_PROTOTYPE_METHOD(t, "getDatasetAttributes", getDatasetAttributes);
        NODE_SET_PROTOTYPE_METHOD(t, "getByteOrder", getByteOrder);
        NODE_SET_PROTOTYPE_METHOD(t, "getFilters", getFilters);

        // initialize constructor reference
        Constructor.Reset(v8::Isolate::GetCurrent(), t);


    }

    void Group::New (const v8::FunctionCallbackInfo<Value>& args) {

//        HandleScope scope;

        // fail out if arguments are not correct
//        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {
//
//            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected name, file")));
//            args.GetReturnValue().SetUndefined();
//            return;
//
//        }
//
        try
        {
        if (args.Length() == 3 && args[0]->IsString() && args[1]->IsObject()) {
            // store specified group name
            String::Utf8Value group_name (args[0]->ToString());
            std::vector<std::string> trail;
            std::vector<hid_t> hidPath;
            std::istringstream buf(*group_name);
            for(std::string token; getline(buf, token, '/'); )
                if(!token.empty())trail.push_back(token);

            hid_t previous_hid;
            // unwrap parent object
            std::string constructorName="File";
            if(constructorName.compare(*String::Utf8Value(args[1]->ToObject()->GetConstructorName()))==0)
            {

                File* parent = ObjectWrap::Unwrap<File>(args[1]->ToObject());
                previous_hid = parent->getId();
            }
            else
            {
               Group* parent = ObjectWrap::Unwrap<Group>(args[1]->ToObject());
                previous_hid = parent->id;
            }
                for(unsigned int index=0;index<trail.size()-1;index++)
                {
                    //check existence of stem
                    if(H5Lexists(previous_hid, trail[index].c_str(), H5P_DEFAULT )){
                        hid_t hid=H5Gopen(previous_hid, trail[index].c_str(), H5P_DEFAULT);
                        if(hid>=0)
                        {
                            if(index<trail.size()-1)hidPath.push_back(hid);
                            previous_hid=hid;
                            continue;
                        }
                        else{
                           std::string msg="Group "+trail[index]+" doesn't exist";
                           v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), msg.c_str())));
                           args.GetReturnValue().SetUndefined();
                           return;
                        }
                    }
                    else{
                           std::string msg="Group "+trail[index]+" doesn't exist";
                           v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), msg.c_str())));
                           args.GetReturnValue().SetUndefined();
                           return;
                    }
                }

               // create group H5Gopen
               Group* group = new Group(H5Gopen(previous_hid, trail[trail.size()-1].c_str(), H5P_DEFAULT));
               group->gcpl_id=H5Pcreate(H5P_GROUP_CREATE);
               herr_t err = H5Pset_link_creation_order(group->gcpl_id, args[2]->ToUint32()->IntegerValue());
               if (err < 0) {
                   v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "Failed to set link creation order")));
                   args.GetReturnValue().SetUndefined();
                   return;
               }
       //        group->m_group.
               group->name.assign(trail[trail.size()-1]);
                for (std::vector<hid_t>::iterator it = hidPath.begin() ; it != hidPath.end(); ++it)
                    group->hidPath.push_back(*it);
               group->Wrap(args.This());


//                v8::Persistent<v8::Object> persistent(v8::Isolate::GetCurrent(), args.This());
//
//                persistent.SetWeak(*args.This(), [](const v8::WeakCallbackData<v8::Object, v8::Object>& data){
//                    //std::cout<<" got a call! "<<std::endl;
//                });

               // attach various properties
                Local<Object> instance=Int64::Instantiate(args.This(), group->id);
                Int64* idWrap = ObjectWrap::Unwrap<Int64>(instance);
                idWrap->value=group->id;
               args.This()->Set(String::NewFromUtf8(v8::Isolate::GetCurrent(), "id"), instance);

        }
        else
        {
                Local<Object> instance=Int64::Instantiate(args.This(), -1);
                Int64* idWrap = ObjectWrap::Unwrap<Int64>(instance);
                idWrap->value=-1;
            args.This()->Set(String::NewFromUtf8(v8::Isolate::GetCurrent(), "id"), instance);

        }
        }
//        catch(H5::GroupIException& ex)
//        {
//            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), ex.getCDetailMsg())));
//            args.GetReturnValue().SetUndefined();
//            return;
//        }
//        catch(H5::FileIException& ex)
//        {
//            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), ex.getCDetailMsg())));
//            args.GetReturnValue().SetUndefined();
//            return;
//        }
        catch(...)
        {
            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "Group open failed")));
            args.GetReturnValue().SetUndefined();
            return;
        }

        return;

    }

    void Group::Create (const v8::FunctionCallbackInfo<Value>& args) {

//        HandleScope scope;

        // fail out if arguments are not correct
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected name, file")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // store specified group name
        String::Utf8Value group_name (args[0]->ToString());

        // unwrap parent object
        File* parent = ObjectWrap::Unwrap<File>(args[1]->ToObject());
        std::vector<std::string> trail;
        std::vector<hid_t> hidPath;
        std::istringstream buf(*group_name);
        for(std::string token; getline(buf, token, '/'); )
            if(!token.empty())trail.push_back(token);
        hid_t previous_hid = parent->getId();
        bool created=false;
        for(unsigned int index=0;index<trail.size();index++)
        {
            //check existence of stem
            if(H5Lexists(previous_hid, trail[index].c_str(), H5P_DEFAULT )){
                hid_t hid=H5Gopen(previous_hid, trail[index].c_str(), H5P_DEFAULT);
                if(hid>=0)
                {
                    if(index<trail.size()-1)hidPath.push_back(hid);
                    previous_hid=hid;
                    continue;
                }
            }
            // create group
//            //std::cout<<previous_hid<<" group create  "<<trail[index]<<" in "<<parent->getFileName()<<std::endl;
            hid_t hid=H5Gcreate(previous_hid, trail[index].c_str(), H5P_DEFAULT, parent->getGcpl(), H5P_DEFAULT);
            if(hid<0){
                //std::cout<<"group create error num "<<H5Eget_num(H5Eget_current_stack())<<std::endl;
                //if(H5Eget_num(H5Eget_current_stack())>0)
                std::string desc;
                {
                    H5Ewalk2(H5Eget_current_stack(), H5E_WALK_UPWARD, [](unsigned int n, const H5E_error2_t *err_desc, void *client_data) -> herr_t {
    //                //std::cout<<"n="<<n<<" "<<err_desc[0].desc<<std::endl;
                    if(((std::string*)client_data)->empty())((std::string*)client_data)->assign(err_desc[0].desc, strlen(err_desc[0].desc));
                    return 0;
                }, (void*)&desc);
                }
                desc="Group create failed: "+desc;
                v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), desc.c_str())));
                args.GetReturnValue().SetUndefined();
                return;
            }
            if(index<trail.size()-1)hidPath.push_back(hid);
            if(index==trail.size()-1)
            {
                Group* group = new Group(hid);
                group->name.assign(trail[index].c_str());
                group->gcpl_id=H5Pcreate(H5P_GROUP_CREATE);
                herr_t err = H5Pset_link_creation_order(group->gcpl_id, args[2]->ToUint32()->IntegerValue());
                if (err < 0) {
                    v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "Failed to set link creation order")));
                    args.GetReturnValue().SetUndefined();
                    return;
                }
                for (std::vector<hid_t>::iterator it = hidPath.begin() ; it != hidPath.end(); ++it)
                    group->hidPath.push_back(*it);
                group->Wrap(args.This());
                Local<Object> instance=Int64::Instantiate(args.This(), group->id);
                Int64* idWrap = ObjectWrap::Unwrap<Int64>(instance);
                idWrap->value=group->id;

                // attach various properties
                args.This()->Set(String::NewFromUtf8(v8::Isolate::GetCurrent(), "id"), instance);
                created=true;
            }
            previous_hid=hid;
        }
        if(!created)
        {
            Group* group = new Group(previous_hid);
            group->name.assign(trail[trail.size()-1].c_str());
            group->gcpl_id=H5Pcreate(H5P_GROUP_CREATE);
            herr_t err = H5Pset_link_creation_order(group->gcpl_id, args[2]->ToUint32()->IntegerValue());
            if (err < 0) {
                v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "Failed to set link creation order")));
                args.GetReturnValue().SetUndefined();
                return;
            }
            for (std::vector<hid_t>::iterator it = hidPath.begin() ; it != hidPath.end(); ++it)
                group->hidPath.push_back(*it);
            group->Wrap(args.This());
                Local<Object> instance=Int64::Instantiate(args.This(), group->id);
                Int64* idWrap = ObjectWrap::Unwrap<Int64>(instance);
                idWrap->value=group->id;

            // attach various properties
            args.This()->Set(String::NewFromUtf8(v8::Isolate::GetCurrent(), "id"), instance);
            created=true;
        }
        return;

    }

    void Group::Open (const v8::FunctionCallbackInfo<Value>& args) {

//        HandleScope scope;

        // fail out if arguments are not correct
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected name, file")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // store specified group name
        String::Utf8Value group_name (args[0]->ToString());

        // unwrap file object
//        File* file = ObjectWrap::Unwrap<File>(args[1]->ToObject());

        std::vector<std::string> trail;
        std::vector<hid_t> hidPath;
        std::istringstream buf(*group_name);
        for(std::string token; getline(buf, token, '/'); )
            if(!token.empty())trail.push_back(token);
        hid_t previous_hid;
        // unwrap parent object
        std::string constructorName="File";
        if(constructorName.compare(*String::Utf8Value(args[1]->ToObject()->GetConstructorName()))==0)
        {

            File* parent = ObjectWrap::Unwrap<File>(args[1]->ToObject());
            previous_hid = parent->getId();
        }
        else
        {
           Group* parent = ObjectWrap::Unwrap<Group>(args[1]->ToObject());
            previous_hid = parent->id;
        }
        for(unsigned int index=0;index<trail.size()-1;index++)
        {
            //check existence of stem
            if(H5Lexists(previous_hid, trail[index].c_str(), H5P_DEFAULT )){
                hid_t hid=H5Gopen(previous_hid, trail[index].c_str(), H5P_DEFAULT);
                if(hid>=0)
                {
                    if(index<trail.size()-1)hidPath.push_back(hid);
                    previous_hid=hid;
                    continue;
                }
                else{
                   std::string msg="Group"+trail[index]+" doesn't exist";
                   v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), msg.c_str())));
                   args.GetReturnValue().SetUndefined();
                   return;
                }
            }
            else{
                   std::string msg="Group"+trail[index]+" doesn't exist";
                   v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), msg.c_str())));
                   args.GetReturnValue().SetUndefined();
                   return;
            }
        }
        Group* group = new Group(H5Gopen(previous_hid, trail[trail.size()-1].c_str(), H5P_DEFAULT));
        group->gcpl_id=H5Pcreate(H5P_GROUP_CREATE);
        herr_t err = H5Pset_link_creation_order(group->gcpl_id, args[2]->ToUint32()->IntegerValue());
        if (err < 0) {
            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "Failed to set link creation order")));
            args.GetReturnValue().SetUndefined();
            return;
        }
        group->name.assign(trail[trail.size()-1]);
        for (std::vector<hid_t>::iterator it = hidPath.begin() ; it != hidPath.end(); ++it)
            group->hidPath.push_back(*it);
        group->Wrap(args.This());
        Local<Object> instance=Int64::Instantiate(args.This(), group->id);
        Int64* idWrap = ObjectWrap::Unwrap<Int64>(instance);
        idWrap->value=group->id;

        // attach various properties
        args.This()->Set(String::NewFromUtf8(v8::Isolate::GetCurrent(), "id"), instance);

        return;

    }

    void Group::OpenGroup (const v8::FunctionCallbackInfo<Value>& args) {

        // fail out if arguments are not correct
        if (args.Length() < 1 || args.Length() >2 || !args[0]->IsString()) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected name")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        String::Utf8Value group_name (args[0]->ToString());

        Local<Object> instance=Group::Instantiate(*group_name, args.This(), args[1]->ToUint32()->Uint32Value());
        // create callback params
//        Local<Value> argv[2] = {
//
//                Local<Value>::New(v8::Isolate::GetCurrent(), Null(v8::Isolate::GetCurrent())),
//                Local<Value>::New(v8::Isolate::GetCurrent(), Group::Instantiate(*group_name, args.This()))
//
//        };

        // execute callback
//        Local<Function> callback = Local<Function>::Cast(args[1]);
//        callback->Call(v8::Isolate::GetCurrent()->GetCurrentContext()->Global(), 2, argv);

        args.GetReturnValue().Set(instance);
        return;

    }

    void Group::Copy (const v8::FunctionCallbackInfo<Value>& args) {
        // fail out if arguments are not correct
        if (args.Length() !=3) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected src name, dest group, dest name")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // unwrap group
        Group* group = ObjectWrap::Unwrap<Group>(args.This());
        String::Utf8Value group_name (args[0]->ToString());
        String::Utf8Value dest_name (args[2]->ToString());
        //std::cout<<*group_name<<" "<<*dest_name<<std::endl;
        herr_t err=H5Ocopy(group->id, *group_name, args[1]->ToUint32()->IntegerValue(), *dest_name, H5P_DEFAULT, H5P_DEFAULT);
        if (err < 0) {
            std::string str(*dest_name);
            std::string errStr="Failed move link to , " + str + " with return: " + std::to_string(err) + ".\n";
            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), errStr.c_str())));
            args.GetReturnValue().SetUndefined();
            return;
        }
    }

    void Group::Move (const v8::FunctionCallbackInfo<Value>& args) {
        // fail out if arguments are not correct
        if (args.Length() !=3) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected src name, dest group, dest name")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // unwrap group
        Group* group = ObjectWrap::Unwrap<Group>(args.This());
        String::Utf8Value group_name (args[0]->ToString());
        String::Utf8Value dest_name (args[2]->ToString());
        //std::cout<<*group_name<<" "<<*dest_name<<std::endl;
        herr_t err=H5Lmove(group->id, *group_name, args[1]->ToUint32()->IntegerValue(), *dest_name, H5P_DEFAULT, H5P_DEFAULT);
        if (err < 0) {
            std::string str(*dest_name);
            std::string errStr="Failed move link to , " + str + " with return: " + std::to_string(err) + ".\n";
            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), errStr.c_str())));
            args.GetReturnValue().SetUndefined();
            return;
        }
    }

    void Group::Link (const v8::FunctionCallbackInfo<Value>& args) {
        // fail out if arguments are not correct
        if (args.Length() !=3) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected src name, dest group, dest name")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // unwrap group
        Group* group = ObjectWrap::Unwrap<Group>(args.This());
        String::Utf8Value group_name (args[0]->ToString());
        String::Utf8Value dest_name (args[2]->ToString());
        //std::cout<<*group_name<<" "<<*dest_name<<std::endl;
        herr_t err=H5Lcopy(group->id, *group_name, args[1]->ToUint32()->IntegerValue(), *dest_name, H5P_DEFAULT, H5P_DEFAULT);
        if (err < 0) {
            std::string str(*dest_name);
            std::string errStr="Failed move link to , " + str + " with return: " + std::to_string(err) + ".\n";
            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), errStr.c_str())));
            args.GetReturnValue().SetUndefined();
            return;
        }
    }

    void Group::Delete (const v8::FunctionCallbackInfo<Value>& args) {
        // fail out if arguments are not correct
        if (args.Length() !=1 || !args[0]->IsString()) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected group name")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // unwrap group
        Group* group = ObjectWrap::Unwrap<Group>(args.This());
        // delete specified group name
        String::Utf8Value group_name (args[0]->ToString());
        H5Ldelete(group->id, (*group_name), H5P_DEFAULT);
    }

    void Group::Close (const v8::FunctionCallbackInfo<Value>& args) {

        // fail out if arguments are not correct
        if (args.Length() >0 ) {

            v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "expected arguments")));
            args.GetReturnValue().SetUndefined();
            return;

        }

        // unwrap group
        Group* group = ObjectWrap::Unwrap<Group>(args.This());
//                if(group->hidPath.size()>0)//std::cout<<"hidPath size: "<<group->hidPath.size()<<std::endl;
        for (std::vector<hid_t>::iterator it = group->hidPath.begin() ; it != group->hidPath.end(); ++it){
//            if(group->hidPath.size()>0)//std::cout<<"closing hid: "<<*it<<std::endl;
            herr_t err=H5Gclose(*it);
            if(err<0)
            {
                //std::cout<<"failed to close group: "<<group->name<<std::endl;
//                v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "failed to close group")));
//                args.GetReturnValue().SetUndefined();
//                return;
            }
        }
        H5Pclose(group->gcpl_id);
        herr_t err=H5Gclose(group->id);
            if(err<0)
            {
                //std::cout<<"failed to close group: "<<group->name<<std::endl;
//                v8::Isolate::GetCurrent()->ThrowException(v8::Exception::SyntaxError(String::NewFromUtf8(v8::Isolate::GetCurrent(), "failed to close group")));
//                args.GetReturnValue().SetUndefined();
                return;
            }

        return;

    }

    Local<Object> Group::Instantiate (Local<Object> file) {

//        HandleScope scope;

        // group name and file reference
        Local<Value> argv[1] = {

                file

        };

        // return new group instance
        return Local<FunctionTemplate>::New(v8::Isolate::GetCurrent(), Constructor)->GetFunction()->NewInstance(1, argv);

    }

    Local<Object> Group::Instantiate (const char* name, Local<Object> parent, unsigned long creationOrder) {

//        HandleScope scope;

        // group name and parent reference
        Local<Value> argv[3] = {

                Local<Value>::New(v8::Isolate::GetCurrent(), String::NewFromUtf8(v8::Isolate::GetCurrent(), name)),
                parent,
                Uint32::New(v8::Isolate::GetCurrent(), creationOrder)

        };

        // return new group instance
        return Local<FunctionTemplate>::New(v8::Isolate::GetCurrent(), Constructor)->GetFunction()->NewInstance(3, argv);

    }

};