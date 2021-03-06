#include <vector>
#include <cmath>
#include <cfloat>

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layer.hpp"
#include "caffe/vision_layers.hpp"
#include "caffe/util/math_functions.hpp"

using std::max;
using std::min;

namespace caffe {

template <typename Dtype>
void MILLayer<Dtype>::LayerSetUp(
		const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {

	CHECK_EQ(bottom.size(), 1) << "MIL Layer takes a single blob as input.";
	CHECK_EQ(top.size(), 1) << "MIL Layer takes a single blob as output.";

	instances_per_bag_ = this->layer_param_.mil_param().instances_per_bag();
	num_bags_ = bottom[0]->num() / instances_per_bag_;
	channels_ = bottom[0]->channels();
	top[0]->Reshape(num_bags_, channels_, 1, 1);
}

template <typename Dtype>
void MILLayer<Dtype>::Reshape(
		const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top){
	instances_per_bag_ = this->layer_param_.mil_param().instances_per_bag();
	num_bags_ = bottom[0]->num() / instances_per_bag_;
	top[0]->Reshape(num_bags_, channels_, 1, 1);
}

template <typename Dtype>
void MILLayer<Dtype>::Forward_cpu(
		const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
	const Dtype* bottom_data = bottom[0]->cpu_data();
	Dtype* top_data = top[0]->mutable_cpu_data();

	// Code to compute the image probabilities from box probabilities

	// For now just substitute the max probability instead of noisy OR
	for(int k = 0; k < channels_; k++){
		for(int i = 0; i < num_bags_; i++){
			Dtype prob;

			prob = -FLT_MAX;
			for(int j = 0; j < instances_per_bag_; j++){
				prob = max(prob, bottom_data[(i*instances_per_bag_ + j)*channels_+k]);
			}
			top_data[i*channels_ + k] = prob;
		}
	}
}

template <typename Dtype>
void MILLayer<Dtype>::Backward_cpu(
		const vector<Blob<Dtype>*>& top, const vector<bool>& propagate_down,
		const vector<Blob<Dtype>*>& bottom) {

	const Dtype* top_diff = top[0]->cpu_diff();
	const Dtype* top_data = top[0]->cpu_data();
	const Dtype* bottom_data = bottom[0]->cpu_data();
	Dtype* bottom_diff = bottom[0]->mutable_cpu_diff();
	if(propagate_down[0]){
		// All the gradient goes to the bow with the probability equal to the
		// probability in the top pf the layer!
		for(int k = 0; k < channels_; k++){
			for(int i = 0; i < num_bags_; i++){
				for(int j = 0; j < instances_per_bag_; j++){

					bottom_diff[(i*instances_per_bag_ + j)*channels_+k] =
							top_diff[i*channels_ + k] *
							(top_data[i*channels_ + k] == bottom_data[(i*instances_per_bag_ + j)*channels_+k]);

				}
			}
		}
	}
}

INSTANTIATE_CLASS(MILLayer);
REGISTER_LAYER_CLASS(MIL);

}  // namespace caffe
